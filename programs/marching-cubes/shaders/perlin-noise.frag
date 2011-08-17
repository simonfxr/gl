uniform mat4 worldMatrix;

in vec3 gTexCoord;

out float fNoise;

/* const int Permu[512] = int[512]( */
/*    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, */
/*    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, */
/*    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, */
/*    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, */
/*    175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, */
/*    229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, */
/*    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, */
/*    89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, */
/*    198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, */
/*    118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, */
/*    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, */
/*    221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, */
/*    110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, */
/*    34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, */
/*    249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, */
/*    176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, */
/*    67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, */
/*    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, */
/*    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, */
/*    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, */
/*    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, */
/*    175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, */
/*    229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, */
/*    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, */
/*    89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, */
/*    198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, */
/*    118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, */
/*    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, */
/*    221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, */
/*    110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, */
/*    34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, */
/*    249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, */
/*    176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, */
/*    67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 */
/*     ); */

int P(int i) {
    if (i < 0) {
        i = 256 - ((-i) & 255);
    } else {
        i &= 255;
    }
    i = 2 * i * i + i;
    return i & 255;
}

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;                      
    float u = h<8 ? x : y;                 
    float v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

float lerp(float t, float a, float b) {
    return a + t * (b - a);
}

int intFloor(float x) {
//    return x > 0 ? int(x) : int(x - 1);
    return int(floor(x));
}
float noise3D(vec3 pnt) {

    float x = pnt.x;
    float y = pnt.y;
    float z = pnt.z;
    
    int X = intFloor(x) & 255,  
        Y = intFloor(y) & 255,
        Z = intFloor(z) & 255;
    
    x -= floor(x);                                
    y -= floor(y);                                
    z -= floor(z);
    
    float u = fade(x);                                
    float v = fade(y);                                
    float w = fade(z);
    
    int A = P(X  )+Y, AA = P(A)+Z, AB = P(A+1)+Z,      
        B = P(X+1)+Y, BA = P(B)+Z, BB = P(B+1)+Z;      

    return lerp(w, lerp(v, lerp(u, grad(P(AA  ), x  , y  , z  ),  
                                   grad(P(BA  ), x-1, y  , z  )), 
                          lerp(u, grad(P(AB  ), x  , y-1, z  ),  
                                grad(P(BB  ), x-1, y-1, z  ))),
                 lerp(v, lerp(u, grad(P(AA+1), x  , y  , z-1),  
                                grad(P(BA+1), x-1, y  , z-1)), 
                       lerp(u, grad(P(AB+1), x  , y-1, z-1),
                             grad(P(BB+1), x-1, y-1, z-1))));
}

void main() {
    float scale = 16;
    mat4 wMatrix = mat4(vec4(scale, 0, 0, 0),
                        vec4(0, scale, 0, 0),
                        vec4(0,0,scale, 0),
                        vec4(0,0,0,1));
                             
    vec4 wc = wMatrix * vec4(gTexCoord, 1);
    fNoise = noise3D(wc.xyz / wc.w);
}
