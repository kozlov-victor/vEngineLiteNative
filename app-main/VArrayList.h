#pragma once

template<class T, size_t SBO_SIZE = 8>
class VArrayList {
private:

    T* Array;
    size_t pointer;
    size_t arrayLength;

    T stackBuffer[SBO_SIZE]; // static buffer object

    bool usingStack() const {
        return Array == (T*)stackBuffer;
    }

    bool needToResize() const {
        return pointer == arrayLength;
    }

    void resize() {
        const size_t newLength = arrayLength * 2;

        T* temp = new T[newLength];

        for (size_t i = 0; i < pointer; i++) {
            temp[i] = Array[i];
        }

        if (!usingStack()) {
            delete[] Array;
        }

        Array = temp;
        arrayLength = newLength;
    }

    bool checkRange(size_t index) const {
        return index < pointer;
    }

public:

    explicit VArrayList(const size_t size = SBO_SIZE) {

        if (size <= SBO_SIZE) {
            Array = stackBuffer;
            arrayLength = SBO_SIZE;
        }
        else {
            Array = new T[size];
            arrayLength = size;
        }

        pointer = 0;
    }

    ~VArrayList() {
        if (!usingStack()) {
            delete[] Array;
        }
    }

    VArrayList(const VArrayList&) = delete;
    VArrayList& operator=(const VArrayList&) = delete;

    // move constructor
    VArrayList(VArrayList&& other) noexcept {

        if (other.usingStack()) {
            Array = stackBuffer;
            arrayLength = SBO_SIZE;

            for (size_t i = 0; i < other.pointer; i++) {
                stackBuffer[i] = other.stackBuffer[i];
            }
        }
        else {
            Array = other.Array;
            arrayLength = other.arrayLength;
            other.Array = nullptr;
        }

        pointer = other.pointer;
        other.pointer = 0;
    }

    // move assignment
    VArrayList& operator=(VArrayList&& other) noexcept {

        if (this != &other) {

            if (!usingStack()) {
                delete[] Array;
            }

            if (other.usingStack()) {

                Array = stackBuffer;
                arrayLength = SBO_SIZE;

                for (size_t i = 0; i < other.pointer; i++) {
                    stackBuffer[i] = other.stackBuffer[i];
                }

            }
            else {

                Array = other.Array;
                arrayLength = other.arrayLength;
                other.Array = nullptr;

            }

            pointer = other.pointer;
            other.pointer = 0;
        }

        return *this;
    }

    void add(const T& a) {
        if (needToResize()) {
            resize();
        }
        Array[pointer++] = a;
    }

    T& getAt(size_t index) {
        if (!checkRange(index)) {
            printf("VArrayList::getAt: index %llu  out of bounds (size %llu)\n", index, pointer);
            return Array[0];
        }

        return Array[index];
    }

    size_t size() const {
        return pointer;
    }

    bool empty() {
        return pointer == 0;
    }

    void removeAt(size_t index) {

        if (!checkRange(index)) return;

        for (size_t i = index; i < pointer - 1; i++) {
            Array[i] = Array[i + 1];
        }

        pointer--;
    }

    void clear() {
        pointer = 0;
        if (!usingStack()) {
            delete[] Array;
        }
        Array = stackBuffer;
    }

    T* begin() { return Array; }
    T* end() { return Array + pointer; }

};
