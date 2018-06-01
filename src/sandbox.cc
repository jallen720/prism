#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <immintrin.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RUN \
    for(size_t pass = 0; pass < PASS_COUNT; pass++) \
    for(size_t i = 0; i < ENTITY_COUNT; i++)

#define RUN_VEC \
    for(size_t pass = 0; pass < PASS_COUNT; pass++) \
    for(size_t i = 0; i < ENTITY_COUNT; i += VECTOR_COUNT)

#define VEC_FOR \
    for(size_t i = 0; i < VECTOR_COUNT; i++)

#define PRISM_ALIGN(BIT_SIZE) __attribute__ ((aligned (BIT_SIZE)))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int32_t elem_t;
static const size_t BITS_PER_BYTE = 8;
static const size_t VECTOR_BIT_SIZE = 256;
static const size_t VECTOR_BYTE_SIZE = VECTOR_BIT_SIZE / BITS_PER_BYTE;
static const size_t ELEM_BIT_SIZE = sizeof(elem_t) * BITS_PER_BYTE;
static const size_t VECTOR_COUNT = VECTOR_BIT_SIZE / ELEM_BIT_SIZE;
static const size_t ENTITY_COUNT = 65536;
static const size_t PASS_COUNT = 144;
static const size_t SIMD_BIT_ALIGNMENT = 32;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DATA
{
    elem_t vs[ENTITY_COUNT] PRISM_ALIGN(SIMD_BIT_ALIGNMENT);
    elem_t ws[ENTITY_COUNT] PRISM_ALIGN(SIMD_BIT_ALIGNMENT);
    elem_t xs[ENTITY_COUNT] PRISM_ALIGN(SIMD_BIT_ALIGNMENT);
    elem_t ys[ENTITY_COUNT] PRISM_ALIGN(SIMD_BIT_ALIGNMENT);
    elem_t zs[ENTITY_COUNT] PRISM_ALIGN(SIMD_BIT_ALIGNMENT);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void print_vector(T * vector, const char * fmt);

void run_test(const char * title, DATA * data);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    DATA data;
    run_test("test", &data);
    return EXIT_SUCCESS ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void print_vector(T * vector, const char * fmt)
{
    for(size_t i = 0; i < VECTOR_COUNT; i++)
    {
        printf(fmt, vector[i]);
        printf(" ");
    }

    puts("");
}

void run_test(const char * title, DATA * data)
{
    clock_t start = 0;
    clock_t end = 0;
    elem_t * vs = data->vs;
    elem_t * ws = data->ws;
    elem_t * xs = data->xs;
    elem_t * ys = data->ys;
    elem_t * zs = data->zs;
    printf("\n%s:\n", title);

#if 1
    start = clock();

    RUN
    {
        vs[i] = ws[i] = xs[i] = ys[i] = zs[i] = 2;
    }

    RUN
    {
        zs[i] = vs[i] + ws[i] + xs[i] + ys[i];
    }

    end = clock();
    printf("standard time: %.2fms\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);
#endif

#if 1
    static __m256i INIT_VECTORS[ENTITY_COUNT / VECTOR_COUNT];
    static __m256i RESULT_VECTORS[ENTITY_COUNT / VECTOR_COUNT];
    static const size_t VECTORS_SIZE = sizeof(INIT_VECTORS);
    start = clock();

    RUN_VEC
    {
        INIT_VECTORS[i / VECTOR_COUNT] = _mm256_setr_epi32(
            0, 1, 2, 3, 4, 5, 6, 7
            // , 8, 9, 10, 11, 12, 13, 14, 15
            );
    }

    memcpy(vs, INIT_VECTORS, VECTORS_SIZE);
    memcpy(ws, INIT_VECTORS, VECTORS_SIZE);
    memcpy(xs, INIT_VECTORS, VECTORS_SIZE);
    memcpy(ys, INIT_VECTORS, VECTORS_SIZE);
    print_vector(vs, "%i");
    print_vector(ws, "%i");
    print_vector(xs, "%i");
    print_vector(ys, "%i");

    RUN_VEC
    {
        __m256i vs_vector = _mm256_load_si256((__m256i const *)(vs + i));
        __m256i ws_vector = _mm256_load_si256((__m256i const *)(ws + i));
        __m256i xs_vector = _mm256_load_si256((__m256i const *)(xs + i));
        __m256i ys_vector = _mm256_load_si256((__m256i const *)(ys + i));

        RESULT_VECTORS[i / VECTOR_COUNT] =
            _mm256_mullo_epi32(
                ys_vector,
                _mm256_mullo_epi32(
                    xs_vector,
                    _mm256_mullo_epi32(
                        vs_vector,
                        ws_vector)));

    }

    memcpy(zs, RESULT_VECTORS, VECTORS_SIZE);
    print_vector(zs, "%i");
    end = clock();
    printf("SIMD time: %.2fms\n", ((double)(end - start) / CLOCKS_PER_SEC) * 1000);
#endif
}
