#include <atomic>
#include <stdexcept>

template<typename T>
class LockFreeArray {
private:
    std::atomic<T>* array;
    std::atomic<int> size;
    int capacity;

public:
    LockFreeArray(int capacity) : capacity(capacity), size(0) {
        array = new std::atomic<T>[capacity];
    }

    ~LockFreeArray() {
        delete[] array;
    }

    bool push(T item) {
        int oldSize, newSize;
        do {
            oldSize = size.load();
            if (oldSize >= capacity) {
                return false;
            }
            newSize = oldSize + 1;
        } while (!size.compare_exchange_weak(oldSize, newSize));

        array[oldSize].store(item);
        return true;
    }

    std::pair<bool, T> pop() {
        int oldSize, newSize;
        do {
            oldSize = size.load();
            if (oldSize <= 0) {
                return {false, T()};
            }
            newSize = oldSize - 1;
        } while (!size.compare_exchange_weak(oldSize, newSize));

        return {true, array[newSize].load()};
    }
};