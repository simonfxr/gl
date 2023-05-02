#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "glt/conf.hpp"
#include "sys/io/Stream.hpp"

#include <bl/new.hpp>
#include <bl/optional.hpp>

#include <unordered_map>

namespace glt {

struct GLT_API Preprocessor : bl::runtime_sized_base<Preprocessor>
{

    struct ContentContext
    {
        Preprocessor &processor;
        const bl::string name;
        bl::string_view data;

        ContentContext(Preprocessor &proc, bl::string &&nam)
          : processor(proc), name(bl::move(nam))
        {}
    };

    struct GLT_API DirectiveContext
    {
        ContentContext content;

        size_t lineLength{};
        size_t lineOffset{};
        size_t beginDirective{}; // size_t of first char in directive
        size_t
          endDirective{}; // size_t of first char behind directive, so length
                          // of directive = endDirective - beginDirective

        DirectiveContext(Preprocessor &proc, bl::string &&name)
          : content(proc, bl::move(name))
        {}
    };

    using DirectiveScopeChange = bool;
    static inline constexpr DirectiveScopeChange beginDirective = true;
    static inline constexpr DirectiveScopeChange endDirective = false;

    static BL_DEF_TAG_CPO(directiveScopeChanged);
    static BL_DEF_TAG_CPO(directiveEncountered);

    using DirectiveHandler = bl::any_ref_t<
      bl::overload<void(bl::self &, bool, const ContentContext &)>(
        directiveScopeChanged),
      bl::overload<void(bl::self &, const DirectiveContext &)>(
        directiveEncountered)>;

    using Handlers = std::unordered_map<bl::string, DirectiveHandler>;
    using HandlerEntry = std::pair<bl::string, DirectiveHandler>;

    static Preprocessor &construct_at(void *) noexcept;
    static Preprocessor &construct_at(void *, sys::io::OutStream) noexcept;

    [[nodiscard]] const bl::string &name() const;
    void name(bl::string &&name);

    void process(bl::string_view);

    DirectiveHandler defaultHandler(DirectiveHandler handler);
    DirectiveHandler defaultHandler() const;

    Handlers &handlers();

    bl::optional<DirectiveHandler> installHandler(bl::string_view directive,
                                                  DirectiveHandler handler);

    sys::io::OutStream out();
    void out(sys::io::OutStream out);

    bool setError();
    bool wasError() const;
    bool clearError();

    BL_DECLARE_RUNTIME_SIZED(Preprocessor, Impl);
};

} // namespace glt

#endif
