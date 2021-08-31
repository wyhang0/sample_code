//
// Created by root on 8/8/21.
//

#ifndef C__11_TEST_ASPECT_H
#define C__11_TEST_ASPECT_H
//　轻量级ＡＯＰ框架
#include <type_traits>

#include "./is_detected.h"
#include "./non_copyable.h"

using namespace std;

HAS_MEMBER_FUNCTION(before)
HAS_MEMBER_FUNCTION(after)

template<typename Func, typename ...Args>
class Aspect : NonCopyable{
public:
    Aspect(Func &&f) : m_func(forward<Func>(f)){}

    template<typename Head, typename ...Tail, typename enable_if<has_member_after<Head, Args...>::value && has_member_before<Head, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, Head&&headAspect, Tail&&...tailAspect){
        headAspect.before(forward<Args>(args)...);
        invoke<Tail...>(forward<Args>(args)..., forward<Tail>(tailAspect)...);
        headAspect.after(forward<Args>(args)...);
    }
    template<typename Head, typename ...Tail, typename enable_if<has_member_after<Head, Args...>::value && !has_member_before<Head, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, Head&&headAspect, Tail&&...tailAspect){
        invoke(forward<Args>(args)..., forward<Tail>(tailAspect)...);
        headAspect.after(forward<Args>(args)...);
    }
    template<typename Head, typename ...Tail, typename enable_if<!has_member_after<Head, Args...>::value && has_member_before<Head, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, Head&&headAspect, Tail&&...tailAspect){
        headAspect.before(forward<Args>(args)...);
        invoke(forward<Args>(args)..., forward<Tail>(tailAspect)...);
    }

    template<typename T, typename enable_if<has_member_after<T, Args...>::value && has_member_before<T, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, T&&t){
        t.before(forward<Args>(args)...);
        m_func(forward<Args>(args)...);
        t.after(forward<Args>(args)...);
    }
    template<typename T, typename enable_if<has_member_after<T, Args...>::value && !has_member_before<T, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, T&&t){
        m_func(forward<Args>(args)...);
        t.after(forward<Args>(args)...);
    }
    template<typename T, typename enable_if<!has_member_after<T, Args...>::value && has_member_before<T, Args...>::value>::type* = nullptr>
    void invoke(Args&&...args, T&&t){
        t.before(forward<Args>(args)...);
        m_func(forward<Args>(args)...);
    }
private:
    Func m_func;
};

template<typename T>
using identify_t = T;

template<typename ...AP, typename Func, typename ...Args>
void invoke(Func&&func, Args&&...args){
    Aspect<Func, Args...> asp{forward<Func>(func)};
    asp.invoke(forward<Args>(args)..., identify_t<AP>()...);
}

#endif //C__11_TEST_ASPECT_H
