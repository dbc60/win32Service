#if !defined(APP_PLATFORM_H)
/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#if !defined(PLATFORM_H)
#include "platform.h"
#endif
#include <string>


class FileHandle
{
private:
    // A generic handle. It could be a HANDLE or HANDLE* on Win32 or a FILE* on Unix-like platforms
    void *handle;
    std::wstring fileName;

    void openPlatformHandle();
    void closePlatformHandle();
    
public:
    // Implement the explicit constructor in the per-platform .cpp file (win32_app.cpp for the Win32
    // implementation)
    explicit FileHandle(std::wstring const& fName)
        : fileName(fName), handle(nullptr)
    {
        void *ptr = 0;
        handle = ptr;
    }
    
    FileHandle() : handle(nullptr)
    {
        void *ptr = 0;

        handle = ptr;
    }

    // Implement the destructor in the per-platform .cpp file
    ~FileHandle()
    {
        closeFile();
    }

    // Use move semantics
    FileHandle(FileHandle&& other)
    {
        handle = other.handle;
        other.handle = 0;
        fileName = other.fileName;
        other.fileName = L"";
    }

    FileHandle& operator=(FileHandle&& other)
    {
        if (this != &other)
        {
            // Release this object's handle
            closeFile();

            // pilfer the other's handle and fileName
            handle = other.handle;
            other.handle = nullptr;
            fileName = std::move(other.fileName);
        }

        return *this;
    }

    void openFile()
    {
        if (!handle)
        {
            this->openPlatformHandle();
        }
    }
    
    void closeFile()
    {
        if (handle)
        {
            this->closePlatformHandle();
        }
    }

    bool isOpen()
    {
        bool result = (handle != nullptr);
        
        return result;
    }

    
    const std::wstring& getFileName()
    {
        return fileName;
    }

};


class BasisDB
{
private:
    FileHandle handleToDB;
    
public:
    // Create a Basis database object
    explicit BasisDB(std::wstring const& dbFileName)
    : handleToDB(dbFileName) {}

    // destructor
    ~BasisDB()
    {
        handleToDB.closeFile();
    }

    // copy constructor is implicitly forbidden due to user-defiend move constructor
    // move constructor
    BasisDB(BasisDB&& other) : handleToDB(std::move(other.handleToDB))
    {
        ;
    }

    // copy assignment is implicitly forbidden due to user-defined move assignment
    // move assignment
    BasisDB& operator=(BasisDB&& other)
    {
        if (this != &other)
        {
            // Release the current object's resources
            handleToDB.closeFile();

            // Pilfer (move) the other's resources
            handleToDB = std::move(other.handleToDB);
        }
        
        return *this;
    }
};


class FileDB
{
};


class FileSignature
{
};


class DeltaFile
{
};


#define APP_PLATFORM_H
#endif
