#ifndef CHESS_V2_ASM_H
#define CHESS_V2_ASM_H
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
    #define ALWAYS_INLINE static inline __attribute__((always_inline))
    #define UNREACHABLE() __builtin_unreachable()
    #define HOT __attribute__((hot))
    #define COLD __attribute__((cold))
    #define LIKELY(exp) __builtin_expect(!!(exp), 1)
    #define UNLIKELY(exp) __builtin_expect(!!(exp), 0)
    #define CONST __attribute__((const)) // no pointers or globals
    #define PURE __attribute__((pure)) // only read from pointers or globals
#elif defined(_MSC_VER)
    #define ALWAYS_INLINE static inline __forceinline
    #define UNREACHABLE() __assume(0)
    #define HOT
    #define COLD
    #define LIKELY(exp) (exp)
    #define UNLIKELY(exp) (exp)
    #define CONST
    #define PURE
#else
    #define ALWAYS_INLINE static inline
    #define UNREACHABLE()
    #define HOT
    #define COLD
    #define LIKELY(exp) (exp)
    #define UNLIKELY(exp) (exp)
    #define CONST
    #define PURE
#endif

#if defined(__POPCNT__) || defined(__BMI__)
    #include <intrin.h>
#endif


ALWAYS_INLINE int POPCNT(uint64_t src) {
    #ifdef __POPCNT__
        return (int)__popcnt64(src);
    #else
        int count = 0;
        for (int i = 0; i < 64; i++) {
            if(src >> i & 1) count++;
        }
        return count;
    #endif
}

ALWAYS_INLINE uint64_t BLSR(uint64_t src) {
    #ifdef __BMI__
        return __blsr_u64(src);
    #else
        return src & (src-1);
    #endif
}

ALWAYS_INLINE int TZCNT(uint64_t num) {
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctzll(num);
    #elif defined(__BMI__) // for msvc etc
        return __tzcnt_u64(num);
    #else
        return 63 + (!num)
            - (((num & -num) & 0x00000000FFFFFFFF) ? 32 : 0)
            - (((num & -num) & 0x0000FFFF0000FFFF) ? 16 : 0)
            - (((num & -num) & 0x00FF00FF00FF00FF) ? 8 : 0)
            - (((num & -num) & 0x0F0F0F0F0F0F0F0F) ? 4 : 0)
            - (((num & -num) & 0x3333333333333333) ? 2 : 0)
            - (((num & -num) & 0x5555555555555555) ? 1 : 0);
    #endif
}

ALWAYS_INLINE uint64_t ANDN(uint64_t negOp, uint64_t op) {
    #ifdef __BMI__
        return __andn_u64(negOp, op);
    #elif defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))
        uint64_t result;
        __asm__("BIC %0, %1, %2" : "=r"(result) : "r"(op), "r"(negOp));
        return result;
    #else
        return ~negOp & op;
    #endif
}

ALWAYS_INLINE uint64_t PEXT(uint64_t src, uint64_t mask) {
    #ifdef __BMI2__
        return _pext_u64(src, mask);
    #else
        uint64_t c = 0, m = 1;
        while (mask) {
            uint64_t b = mask & -mask;
            if (src & b)
                c |= m;
            mask -= b;
            m <<= 1;
        }
        return c;
    #endif
}


ALWAYS_INLINE uint64_t PDEP(uint64_t src, uint64_t mask) {
    #ifdef __BMI2__
        return _pdep_u64(src, mask);
    #else
        uint64_t c = 0, m = 1;
        while (mask) {
            uint64_t b = mask & -mask;
            if (src & m)
                c |= b;
            mask -= b;
            m <<= 1;
        }
        return c;
    #endif
}


#endif //CHESS_V2_ASM_H
