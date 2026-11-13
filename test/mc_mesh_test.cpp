// Manual, headless OpenCL regression: build/run mc_mesh_test, not part of CTest.
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include <CL/opencl.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

constexpr int chunk_size = 128, domain_size = 256;
constexpr double spacing = 1.0 / chunk_size, position_tolerance = 2e-7;
constexpr double normal_tolerance = 2e-6;
constexpr float sentinel = -12345.25f;
constexpr int cell_sentinel = -123456789;
using Point = std::array<double, 3>;
// Canonical source-grid edge: lower endpoint and direction (3 means a grid point).
using Edge = std::array<int, 4>;

void
require(bool passed, const char *message)
{
    if (!passed) {
        std::cerr << "mc_mesh_test: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void
checkCL(cl_int status, const char *operation)
{
    if (status != CL_SUCCESS) {
        std::cerr << "mc_mesh_test: " << operation << " failed (OpenCL "
                  << status << ")\n";
        std::exit(EXIT_FAILURE);
    }
}

void
inject(std::string &source, const std::string &before, const std::string &after)
{
    const auto offset = source.find(before);
    require(offset != std::string::npos, "kernel instrumentation anchor changed");
    require(source.find(before, offset + before.size()) == std::string::npos,
            "kernel instrumentation anchor is not unique");
    source.replace(offset, before.size(), after);
}

struct Vertex {
    Point position, normal;
    Edge edge;
};
struct Triangle {
    std::array<Vertex, 3> vertices;
    std::array<int, 3> cell;
};
using Mesh = std::vector<Triangle>;

struct Runner {
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;
    std::string source;

    bool initialize()
    {
        std::vector<cl::Platform> platforms;
        const auto status = cl::Platform::get(&platforms);
        if (status == -1001) // CL_PLATFORM_NOT_FOUND_KHR
            return false;
        checkCL(status, "enumerate platforms");
        const std::array<cl::ImageFormat, 5> required = {
          cl::ImageFormat(CL_R, CL_HALF_FLOAT),
          cl::ImageFormat(CL_RG, CL_UNSIGNED_INT8),
          cl::ImageFormat(CL_R, CL_UNSIGNED_INT8),
          cl::ImageFormat(CL_R, CL_UNSIGNED_INT16),
          cl::ImageFormat(CL_R, CL_UNSIGNED_INT32)};
        for (const auto &platform : platforms) {
            std::vector<cl::Device> devices;
            auto result = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            if (result == CL_DEVICE_NOT_FOUND)
                continue;
            checkCL(result, "enumerate devices");
            for (const auto &candidate : devices) {
                cl_bool images = CL_FALSE, compiler = CL_FALSE;
                size_t width = 0, height = 0, depth = 0;
                checkCL(candidate.getInfo(CL_DEVICE_IMAGE_SUPPORT, &images), "query images");
                checkCL(candidate.getInfo(CL_DEVICE_COMPILER_AVAILABLE, &compiler), "query compiler");
                checkCL(candidate.getInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, &width), "query image width");
                checkCL(candidate.getInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, &height), "query image height");
                checkCL(candidate.getInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, &depth), "query image depth");
                if (!images || !compiler || std::min({width, height, depth}) < domain_size + 3)
                    continue;
                cl_int error;
                cl::Context trial(candidate, nullptr, nullptr, nullptr, &error);
                if (error != CL_SUCCESS)
                    continue;
                std::vector<cl::ImageFormat> formats;
                checkCL(trial.getSupportedImageFormats(CL_MEM_READ_WRITE,
                          CL_MEM_OBJECT_IMAGE3D, &formats), "query image formats");
                const bool supported = std::all_of(required.begin(), required.end(),
                  [&](const auto &format) {
                      return std::any_of(formats.begin(), formats.end(), [&](const auto &available) {
                          return format.image_channel_order == available.image_channel_order &&
                                 format.image_channel_data_type == available.image_channel_data_type;
                      });
                  });
                if (!supported)
                    continue;
                device = candidate;
                context = trial;
                queue = cl::CommandQueue(context, device, 0, &error);
                checkCL(error, "create queue");
                std::string name;
                checkCL(device.getInfo(CL_DEVICE_NAME, &name), "query device name");
                std::cout << "mc_mesh_test: " << name << '\n';
                return true;
            }
        }
        return false;
    }

    void loadSource()
    {
        std::ifstream file(std::string(MC_SOURCE_ROOT) + "/programs/mc/cl/program.cl");
        require(bool(file), "cannot open program.cl");
        source.assign(std::istreambuf_iterator<char>(file), {});
        // Add metadata stores only: traversal, interpolation and normal arithmetic
        // remain the real program.cl implementation. Keep sum as the last argument.
        inject(source, "__private int sum)",
               "__global int4 *testCells, __global int4 *testEdges, __private int sum)");
        inject(source, "    char vertexNr = 0;",
               "    testCells[target] = cubePosition;\n    char vertexNr = 0;");
        inject(source, "        vstore3(vertex,", R"CL(
        int direction = p0.x != p1.x ? 0 : (p0.y != p1.y ? 1 : 2);
        int4 key = (int4)(p0, direction);
        if (diff == 0.f) key = (int4)(p0, 3);
        if (diff == 1.f) key = (int4)(p1, 3);
        testEdges[target * 3 + vertexNr] = key;
        vstore3(vertex,)CL");
    }

    cl::Program build(int size)
    {
        cl_int error;
        cl::Program program(context, source, false, &error);
        checkCL(error, "create program");
        const auto options = std::string("-I \"") + MC_SOURCE_ROOT +
                             "/programs/mc\" -DSIZE=" + std::to_string(size);
        error = program.build({device}, options.c_str());
        if (error != CL_SUCCESS) {
            std::string log;
            checkCL(program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &log), "read build log");
            std::cerr << log << '\n';
            checkCL(error, "build program");
        }
        return program;
    }

    void append(Mesh &mesh, const cl::Program &program, int size,
                std::array<int, 3> origin, double time)
    {
        cl_int error;
        cl::Image3D volume(context, CL_MEM_READ_WRITE, {CL_R, CL_HALF_FLOAT},
                           size + 3, size + 3, size + 3, 0, 0, nullptr, &error);
        checkCL(error, "create volume");
        cl_float16 transform{};
        transform.s[0] = transform.s[5] = transform.s[10] = spacing;
        transform.s[15] = 1;
        for (int axis = 0; axis < 3; ++axis)
            transform.s[12 + axis] = -1 + origin[axis] * spacing;
        time *= 0.5;
        const double phase = std::cos(time) * 0.1 / (2 * std::acos(-1.));
        cl_float4 c{};
        c.s[0] = -std::abs(2 * (phase - std::floor(phase)) - 1);
        c.s[1] = 0.2 * std::cos(time * 0.11);
        c.s[2] = std::sin(time * 0.1);
        cl::Kernel generate(program, "generateSphereVolume", &error);
        checkCL(error, "create generation kernel");
        checkCL(generate.setArg(0, volume), "set volume");
        checkCL(generate.setArg(1, transform), "set transform");
        checkCL(generate.setArg(2, c), "set C");
        checkCL(queue.enqueueNDRangeKernel(generate, cl::NullRange,
                  cl::NDRange(size + 3, size + 3, size + 3)), "generate volume");

        std::vector<cl::Image3D> histogram;
        histogram.emplace_back(context, CL_MEM_READ_WRITE,
          cl::ImageFormat(CL_RG, CL_UNSIGNED_INT8), size, size, size, 0, 0, nullptr, &error);
        checkCL(error, "create base histogram");
        unsigned maximum = 5;
        for (int width = size / 2; width > 1; width /= 2) {
            maximum *= 8;
            const auto type = maximum >= 65536 ? CL_UNSIGNED_INT32 :
                              maximum >= 256 ? CL_UNSIGNED_INT16 : CL_UNSIGNED_INT8;
            histogram.emplace_back(context, CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, type),
                                    width, width, width, 0, 0, nullptr, &error);
            checkCL(error, "create histogram level");
        }
        cl::Kernel base(program, "computeHistogramBaseLevel", &error);
        checkCL(error, "create base kernel");
        checkCL(base.setArg(0, histogram[0]), "set base histogram");
        checkCL(base.setArg(1, volume), "set base volume");
        checkCL(queue.enqueueNDRangeKernel(base, cl::NullRange, cl::NDRange(size, size, size)),
                "compute base histogram");
        cl::Kernel level(program, "computeHistogramLevel", &error);
        checkCL(error, "create histogram kernel");
        int width = size / 2;
        for (size_t i = 1; i < histogram.size(); ++i, width /= 2) {
            checkCL(level.setArg(0, histogram[i - 1]), "set histogram source");
            checkCL(level.setArg(1, histogram[i]), "set histogram destination");
            checkCL(queue.enqueueNDRangeKernel(level, cl::NullRange,
                      cl::NDRange(width, width, width)), "reduce histogram");
        }
        std::array<cl_uint, 8> top{};
        checkCL(queue.enqueueReadImage(histogram.back(), CL_TRUE, {0, 0, 0}, {2, 2, 2},
                                       0, 0, top.data()), "read histogram count");
        cl_int count = 0;
        for (auto value : top)
            count += value;
        require(count > 0, "unexpected empty mesh");
        const size_t padded = (size_t(count) / 64 + 1) * 64;
        std::vector<float> output(padded * 18, sentinel);
        cl_int4 empty{};
        for (int &value : empty.s)
            value = cell_sentinel;
        std::vector<cl_int4> cells(padded, empty), edges(padded * 3, empty);
        cl::Buffer vertices(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                             output.size() * sizeof(float), output.data(), &error);
        checkCL(error, "create vertex buffer");
        cl::Buffer cellBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                               cells.size() * sizeof(cl_int4), cells.data(), &error);
        checkCL(error, "create cell buffer");
        cl::Buffer edgeBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                               edges.size() * sizeof(cl_int4), edges.data(), &error);
        checkCL(error, "create edge buffer");
        cl::Kernel traversal(program, "histogramTraversal", &error);
        checkCL(error, "create traversal kernel");
        unsigned argument = 0;
        for (const auto &image : histogram)
            checkCL(traversal.setArg(argument++, image), "set traversal histogram");
        checkCL(traversal.setArg(argument++, volume), "set traversal volume");
        checkCL(traversal.setArg(argument++, vertices), "set traversal output");
        checkCL(traversal.setArg(argument++, cellBuffer), "set cell metadata");
        checkCL(traversal.setArg(argument++, edgeBuffer), "set edge metadata");
        checkCL(traversal.setArg(argument++, count), "set triangle count");
        // An excess-only launch detects even target=0 aliasing, which a regular
        // padded launch can conceal by writing identical data in a race.
        checkCL(queue.enqueueNDRangeKernel(traversal, cl::NDRange(count),
                  cl::NDRange(padded - count)), "launch excess IDs only");
        checkCL(queue.enqueueReadBuffer(vertices, CL_TRUE, 0,
                  output.size() * sizeof(float), output.data()), "read excess-only output");
        require(std::all_of(output.begin(), output.end(), [](float x) { return x == sentinel; }),
                "excess IDs wrote to the sentinel-filled vertex buffer");
        checkCL(queue.enqueueNDRangeKernel(traversal, cl::NullRange,
                  cl::NDRange(padded), cl::NDRange(64)), "launch padded traversal");
        checkCL(queue.enqueueReadBuffer(vertices, CL_TRUE, 0,
                  output.size() * sizeof(float), output.data()), "read vertices");
        checkCL(queue.enqueueReadBuffer(cellBuffer, CL_TRUE, 0,
                  cells.size() * sizeof(cl_int4), cells.data()), "read cells");
        checkCL(queue.enqueueReadBuffer(edgeBuffer, CL_TRUE, 0,
                  edges.size() * sizeof(cl_int4), edges.data()), "read edge metadata");
        require(std::all_of(output.begin() + size_t(count) * 18, output.end(),
                            [](float x) { return x == sentinel; }), "padded IDs wrote vertex tail");
        for (size_t i = count; i < padded; ++i)
            for (int axis = 0; axis < 4; ++axis) {
                require(cells[i].s[axis] == cell_sentinel, "padded IDs wrote cell metadata");
                for (int v = 0; v < 3; ++v)
                    require(edges[i * 3 + v].s[axis] == cell_sentinel,
                            "padded IDs wrote edge metadata");
            }
        for (int i = 0; i < count; ++i) {
            Triangle triangle{};
            for (int axis = 0; axis < 3; ++axis) {
                require(cells[i].s[axis] >= 0 && cells[i].s[axis] < size, "invalid owning cell");
                triangle.cell[axis] = origin[axis] + cells[i].s[axis];
            }
            for (int v = 0; v < 3; ++v) {
                auto &vertex = triangle.vertices[v];
                double normal_squared = 0;
                for (int axis = 0; axis < 3; ++axis) {
                    const float p = output[size_t(i) * 18 + v * 6 + axis];
                    const float n = output[size_t(i) * 18 + v * 6 + 3 + axis];
                    require(std::isfinite(p) && std::isfinite(n), "nonfinite position or normal");
                    require(p >= cells[i].s[axis] - 2e-5 && p <= cells[i].s[axis] + 1 + 2e-5,
                            "vertex outside owning cell");
                    vertex.position[axis] = -1 + (origin[axis] + double(p)) * spacing;
                    vertex.normal[axis] = n;
                    normal_squared += double(n) * n;
                    vertex.edge[axis] = origin[axis] + edges[i * 3 + v].s[axis];
                }
                require(std::abs(normal_squared - 1) < 2e-4, "normal is not unit length");
                vertex.edge[3] = edges[i * 3 + v].s[3];
            }
            std::sort(triangle.vertices.begin(), triangle.vertices.end(),
                      [](const auto &a, const auto &b) { return a.edge < b.edge; });
            mesh.push_back(triangle);
        }
    }
};

void
compareVertex(const Vertex &a, const Vertex &b)
{
    require(a.edge == b.edge, "canonical source-grid vertex mismatch");
    for (int axis = 0; axis < 3; ++axis) {
        require(std::abs(a.position[axis] - b.position[axis]) <= position_tolerance,
                "world-space position mismatch");
        require(std::abs(a.normal[axis] - b.normal[axis]) <= normal_tolerance,
                "normal mismatch/discontinuity");
    }
}

void
compareMeshes(Mesh &chunked, Mesh &monolithic)
{
    require(chunked.size() == monolithic.size(), "triangle counts differ");
    const auto order = [](const auto &a, const auto &b) {
        if (a.cell != b.cell)
            return a.cell < b.cell;
        for (int v = 0; v < 3; ++v)
            if (a.vertices[v].edge != b.vertices[v].edge)
                return a.vertices[v].edge < b.vertices[v].edge;
        return false;
    };
    std::sort(chunked.begin(), chunked.end(), order);
    std::sort(monolithic.begin(), monolithic.end(), order);
    for (size_t i = 0; i < chunked.size(); ++i) {
        require(chunked[i].cell == monolithic[i].cell, "owning-cell mismatch");
        for (int v = 0; v < 3; ++v)
            compareVertex(chunked[i].vertices[v], monolithic[i].vertices[v]);
    }
    // Also compare all occurrences of shared source-grid edges, including across
    // chunk joins, rather than relying only on chunk-to-monolith correspondence.
    std::vector<const Vertex *> shared;
    shared.reserve(chunked.size() * 3);
    for (const auto &triangle : chunked)
        for (const auto &vertex : triangle.vertices)
            shared.push_back(&vertex);
    std::sort(shared.begin(), shared.end(), [](auto a, auto b) { return a->edge < b->edge; });
    for (size_t i = 1; i < shared.size(); ++i)
        if (shared[i - 1]->edge == shared[i]->edge)
            compareVertex(*shared[i - 1], *shared[i]);
}

void
checkTopology(const Mesh &mesh)
{
    using Segment = std::array<Edge, 2>;
    std::vector<Segment> segments;
    segments.reserve(mesh.size() * 3);
    for (const auto &triangle : mesh) {
        const auto &v = triangle.vertices;
        // Exact isovalue grid points can collapse a triangle to a segment.
        if (v[0].edge == v[1].edge || v[1].edge == v[2].edge || v[0].edge == v[2].edge)
            continue;
        for (int i = 0; i < 3; ++i) {
            Segment segment{v[i].edge, v[(i + 1) % 3].edge};
            if (segment[1] < segment[0])
                std::swap(segment[0], segment[1]);
            segments.push_back(segment);
        }
    }
    std::sort(segments.begin(), segments.end());
    size_t unmatched = 0, nonmanifold = 0;
    for (size_t i = 0; i < segments.size();) {
        size_t end = i + 1;
        while (end < segments.size() && segments[end] == segments[i])
            ++end;
        bool outer = false;
        for (int axis = 0; axis < 3; ++axis) {
            const auto &a = segments[i][0], &b = segments[i][1];
            outer |= a[axis] == b[axis] && (a[axis] == 0 || a[axis] == domain_size) &&
                     a[3] != axis && b[3] != axis;
        }
        if (!outer) {
            unmatched += end - i == 1;
            nonmanifold += end - i > 2;
        }
        i = end;
    }
    std::cout << "; interior unmatched edges=" << unmatched
              << ", nonmanifold edges=" << nonmanifold;
    require(unmatched == 0, "mesh has unmatched interior edges");
    require(nonmanifold == 0, "mesh has nonmanifold interior edges");
}

} // namespace

int
main()
{
    Runner runner;
    if (!runner.initialize()) {
        std::cerr << "mc_mesh_test: unsupported setup: requires an OpenCL device/compiler "
                     "with 259^3 images and half/unsigned histogram formats (exit 77)\n";
        return 77;
    }
    runner.loadSource();
    const auto small = runner.build(chunk_size), large = runner.build(domain_size);
    for (double time : {0., 4., 12.}) {
        Mesh chunked, monolithic;
        for (int chunk = 0; chunk < 8; ++chunk)
            runner.append(chunked, small, chunk_size,
                          {chunk_size * (chunk & 1), chunk_size * ((chunk >> 1) & 1),
                           chunk_size * ((chunk >> 2) & 1)}, time);
        runner.append(monolithic, large, domain_size, {0, 0, 0}, time);
        compareMeshes(chunked, monolithic);
        std::cout << "time=" << time << ": " << monolithic.size()
                  << " triangles; geometry/normals, finite/bounds, padding PASS";
        checkTopology(monolithic);
        std::cout << '\n';
    }
    return EXIT_SUCCESS;
}
