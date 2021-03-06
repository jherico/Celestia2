//
//  Created by Bradley Austin Davis on 2016/02/17
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

namespace storage {

#if defined(__ANDROID__)
void setAssetManager(AAssetManager* assetManager);
#endif

class Storage;
using StoragePointer = std::shared_ptr<const Storage>;
using ByteArray = std::vector<uint8_t>;

// Abstract class to represent memory that stored _somewhere_ (in system memory or in a file, for example)
class Storage : public std::enable_shared_from_this<Storage> {
public:
    virtual ~Storage() {}
    virtual const uint8_t* data() const = 0;
    virtual size_t size() const = 0;
    virtual bool isFast() const = 0;

    static StoragePointer create(size_t size, uint8_t* data);
    static StoragePointer readFile(const std::string& filename);
    StoragePointer createView(size_t size = 0, size_t offset = 0) const;

    // Aliases to prevent having to re-write a ton of code
    inline size_t getSize() const { return size(); }
    inline const uint8_t* readData() const { return data(); }
};

struct IncrementalStorage {
    IncrementalStorage(const StoragePointer& storage, size_t offset = 0) : storage(storage), offset(offset) {}

    IncrementalStorage(const std::string& filename) : IncrementalStorage(Storage::readFile(filename)) {}

    using Pointer = std::shared_ptr<IncrementalStorage>;

    void read(char* dest, size_t size) {
        memcpy(dest, storage->data() + offset, size);
        offset += size;
    }

    void read(uint8_t* dest, size_t size) {
        memcpy(dest, storage->data() + offset, size);
        offset += size;
    }

    int get() {
        uint8_t nextChar = storage->data()[offset];
        ++offset;
        return nextChar;
    }

    bool eof() const { return offset >= storage->size(); }

    IncrementalStorage& ignore(size_t size) {
        offset += size;
        return *this;
    }

    size_t tellg() const { return offset; }

private:
    StoragePointer storage;
    size_t offset{ 0 };
};

}  // namespace storage
