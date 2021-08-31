//
// Created by root on 7/27/21.
//

#ifndef C__11_TEST_IS_DETECTED_H
#define C__11_TEST_IS_DETECTED_H

using namespace std;

#if !(__cplusplus >= 201703L)
template <typename... T>
struct make_void {
    using type = void;
};
template <typename... T>
using void_t = typename make_void<T...>::type;
#endif

//template <typename T, typename = void>
//struct has_get : std::false_type {};
//template <typename T>
//struct has_get<T, void_t<decltype(std::declval<T&>().get())>> : std::true_type {};

#if 1
#define HAS_MEMBER_FUNCTION(function) \
template<typename T, typename...Args> \
struct has_member_##function { \
private: \
    template<typename U> \
    static false_type check(...); \
    template<typename U> \
    static auto check(int) -> decltype(declval<U>().function(declval<Args>()...), true_type()); \
public: \
    static constexpr bool value = is_same<decltype(check<T>(1)), true_type>::value; \
};

#define HAS_MEMBER_TYPE(type) \
template<typename T, typename Enable=void> \
struct has_member_##type : false_type {}; \
template<typename T> \
struct has_member_##type<T, void_t<typename T::type>> : true_type {};

#define HAS_MEMBER_VAL(val) \
template<typename T, typename Enable=void> \
struct has_member_##val : false_type {}; \
template<typename T> \
struct has_member_##val<T, void_t<decltype(declval<T>().val)>> : true_type {};
#else
#define HAS_MEMBER_FUNCTION(function) \
template<typename T, typename...Args> \
struct has_member_##function { \
private: \
    template<typename U> \
    static false_type check(...); \
    template<typename U> \
    static auto check(int) -> decltype(declval<U>().function(declval<Args>()...), true_type()); \
public: \
    enum { \
        value = is_same<decltype(check<T>(1)), true_type>::value \
    }; \
};

#define HAS_MEMBER_TYPE(type) \
template<typename T> \
struct has_member_##type { \
private: \
    template<typename U> \
    static false_type check(...); \
    template<typename U> \
    static auto check(int) -> decltype(declval<typename U::type>(), true_type()); \
public: \
    enum { \
        value = is_same<decltype(check<T>(1)), true_type>::value \
    }; \
};

#define HAS_MEMBER_VAL(val) \
template<typename T> \
struct has_member_##val { \
private: \
    template<typename U> \
    static false_type check(...); \
    template<typename U> \
    static auto check(int) -> decltype(declval<U>().val, true_type()); \
public: \
    enum { \
        value = is_same<decltype(check<T>(1)), true_type>::value \
    }; \
};
#endif

template <typename, template <typename...> class Op, typename... T>
struct is_detected_impl : std::false_type {};
template <template <typename...> class Op, typename... T>
struct is_detected_impl<void_t<Op<T...>>, Op, T...> : std::true_type {};

template <template <typename...> class Op, typename... T>
using is_detected = is_detected_impl<void, Op, T...>;

//template <typename T>
//using has_get_t = decltype(std::declval<T>().get());
//template <typename T>
//using has_get = is_detected<has_get_t, T>;

#endif //C__11_TEST_IS_DETECTED_H
