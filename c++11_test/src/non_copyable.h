//
// Created by root on 8/3/21.
//

#ifndef C__11_TEST_NON_COPYABLE_H
#define C__11_TEST_NON_COPYABLE_H

class NonCopyable{
protected:
    NonCopyable()=default;
    virtual ~NonCopyable()=default;
    NonCopyable(const NonCopyable&)=delete;
    NonCopyable& operator=(const NonCopyable&)=delete;
};

#endif //C__11_TEST_NON_COPYABLE_H
