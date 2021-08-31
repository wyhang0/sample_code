//
// Created by root on 7/15/21.
//

#include "./src/student.h"
#include "./src/syncqueue/sync_stl_queue.h"
#include "./src/syncqueue/sync_ring_array_v1.h"
#include "./src/syncqueue/sync_ring_array_v2.h"
#include "./src/timer.h"

#include <cassert>
#include <iostream>
#include <future>
#include <chrono>

#include <unordered_map>

using namespace std;
using namespace chrono;

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<int>>::value>::type>
void testIntType(){
    Q q;
    q.push(23);
    q.push(21);

    int i;
    q.pop(i);
    assert(i==23);
    q.pop(i);
    assert(i==21);
}
template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<Student>>::value>::type>
void testStudentType(){
    Q q;
    q.emplace(microseconds{250}, string{"wy"}, 18, 100);
    q.emplace(microseconds{250},string{"zs"}, 20, 90);

    Student s;
    q.pop(s);
    assert(s.name=="wy");
    q.pop(s);
    assert(s.name=="zs");
}

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<shared_ptr<int>>>::value>::type>
void testSharePtrType() {
    Q q;
    q.push(make_shared<int>(1));
    q.push(make_shared<int>(2));

    shared_ptr<int> i;
    q.pop(i);
    assert(*i == 1);
    q.pop(i);
    assert(*i == 2);
}

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<int*>>::value>::type>
void testPointType(){
    Q q;
    q.push(new int{1});
    q.push(new int{2});

    int *i;
    q.pop(&i);
    assert(*i==1);
    delete i;
    q.pop(&i);
    assert(*i==2);
    delete i;
}

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<Student*>>::value>::type>
void testStudentPointType(){
    Q q;
    q.emplace(microseconds{250}, new Student{"wy", 18, 100});
    q.emplace(microseconds{250}, new Student{"zs", 20, 90});

    Student *s;
    q.pop(&s);
    assert(s->name=="wy");
    delete s;
    q.pop(&s);
    assert(s->name=="zs");
    delete s;
}

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<Student>>::value>::type>
void testProducerConsumerStudent(string name){
    clog<<name<<endl;

    Q queue;
    int repeat = 5;
    int costTime = 0;
    for (int i = 0; i < repeat; i++) {
        long long len = 10000, count = 5;
        long long value = 0;// = (len+1)*count*(len/2);
        auto producer = [&]{
            for (int i = 0; i < len; ++i) {
                //自旋操作,测试极限性能
                while(!queue.emplace(microseconds{250}, "se", 1, move(i+1)));
            }
        };
        auto consumer = [&]{
            Student student;
            long long value = 0;
            for (int i = 0; i < len; ++i) {
                //自旋操作,测试极限性能
                while(!queue.pop(student));
                value += student.score;
            }
            return value;
        };

        auto begin = system_clock::now();

        future<void> produces[count];
        future<long long> consumes[count];
        for (int i = 0; i < count; ++i) {
            produces[i] = async(launch::async, producer);
            consumes[i] = async(launch::async, consumer);
        }

        for (int i = 0; i < count; ++i) {
            produces[i].wait();
            value += consumes[i].get();
        }

        assert(value == (len+1)*(len/2)*count);
        auto dur = duration_cast<milliseconds>(system_clock::now()-begin).count();
        costTime += dur;
        clog << dur << endl;
    }
    clog <<"平均 " << costTime/repeat << endl;
}

template<typename Q, typename = typename enable_if<is_convertible<Q, SyncQueue<long long>>::value>::type>
void testProducerConsumerLongLong(string name){
    clog<<name<<endl;
    Q queue;
    int repeat = 5;
    int costTime = 0;
    for (int i = 0; i < repeat; i++) {
        long long len = 10000, count = 5;
        long long value = 0;// = (len+1)*count*(len/2);
        auto producer = [&]{
            for (int i = 0; i < len; ++i) {
                //自旋操作,测试极限性能
                while(!queue.push(i+1));
            }
        };
        auto consumer = [&]{
            long long tmp;
            long long value = 0;
            for (int i = 0; i < len; ++i) {
                //自旋操作,测试极限性能
                while(!queue.pop(tmp));
                value += tmp;
            }
            return value;
        };

        auto getQueueSize = [&]{
            while (value != (len+1)*(len/2)*count){
                cout << "size " << queue.size()<< endl;
                this_thread::sleep_for(milliseconds(1));
            }
        };

        auto begin = system_clock::now();

        future<void> produces[count];
        future<long long> consumes[count];
        for (int i = 0; i < count; ++i) {
            produces[i] = async(launch::async, producer);
            consumes[i] = async(launch::async, consumer);
        }
//        auto execGetQueueSize = async(launch::async, getQueueSize);

        for (int i = 0; i < count; ++i) {
            produces[i].wait();
            value += consumes[i].get();
        }
//        execGetQueueSize.wait();
        cout.flush();
        auto dur = duration_cast<milliseconds>(system_clock::now()-begin).count();
        costTime += dur;
        clog << dur << endl;
        assert(value == (len+1)*(len/2)*count);
    }
    clog << "平均 " << costTime/repeat << endl;
}

void testOneToOne(){
    //1:1 生产者消费者模型
    int value = 0;
    atomic_bool flag{false};

    auto produce = [&](int i){
        int count = 7000000;
        while(count--){
            while(flag.load(memory_order_relaxed)){
//                this_thread::sleep_for(microseconds {250});
            }
//            cout << "A" << i << " " << m_value << endl;
            value++;
            flag.store(true, memory_order_release);
        }
    };
    auto consume = [&](int i){
        int count = 7000000;
        while(count--){
            while(!flag.load(memory_order_acquire)){
//                this_thread::sleep_for(microseconds{250});
            }
//            cout << "B" << i << " " << m_value << endl;
            value++;
            flag.store(false, memory_order_relaxed);
        }
    };

    auto start = system_clock::now();
    size_t count = 1;
    vector<shared_future<void>> vector{count*2};

    for (int i = 0; i < count; ++i) {
        vector[i] = {async(launch::async, produce, i)};
        vector[i+count] = {async(launch::async, consume, i)};
    }
    for (auto & i : vector) {
        i.wait();
    }
    cout << duration_cast<milliseconds>(system_clock::now()-start).count() << endl;
    cout << value/2 << endl;
}

template<typename T>
struct AA{
    aligned_storage<sizeof(T), alignof(T)> t;
};

int main(){
    testIntType<SyncStlQueue<int>>();
    testStudentType<SyncStlQueue<Student>>();
    testPointType<SyncStlQueue<int*>>();
    testStudentPointType<SyncStlQueue<Student*>>();
    testSharePtrType<SyncStlQueue<shared_ptr<int>>>();

    testIntType<SyncRingArrayV1<int>>();
    testStudentType<SyncRingArrayV1<Student>>();
    testPointType<SyncRingArrayV1<int*>>();
    testStudentPointType<SyncRingArrayV1<Student*>>();
    testSharePtrType<SyncRingArrayV1<shared_ptr<int>>>();

    testIntType<SyncRingArrayV2<int>>();
    testStudentType<SyncRingArrayV2<Student>>();
    testPointType<SyncRingArrayV2<int*>>();
    testStudentPointType<SyncRingArrayV2<Student*>>();
    testSharePtrType<SyncRingArrayV2<shared_ptr<int>>>();

    auto rdbuf = cout.rdbuf();
    cout.rdbuf(nullptr);
    testProducerConsumerStudent<SyncStlQueue<Student>>("SyncStlQueue<Student>");
    testProducerConsumerStudent<SyncRingArrayV1<Student>>("SyncRingArrayV1<Student>");
    testProducerConsumerStudent<SyncRingArrayV2<Student>>("SyncRingArrayV2<Student>");
    cout.rdbuf(rdbuf);

    testOneToOne();

    testProducerConsumerLongLong<SyncStlQueue<long long>>("SyncStlQueue<long long>");
    testProducerConsumerLongLong<SyncRingArrayV1<long long>>("SyncRingArrayV1<long long>");
    Timer timer;
    testProducerConsumerLongLong<SyncRingArrayV2<long long>>("SyncRingArrayV2<long long>");
    cout << timer.elapsed() << endl;

    return 0;
}

