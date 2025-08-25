#include "fileholder.h"
#include <QFileInfo>
#include <QDebug>

#ifdef Q_OS_WIN
#include <fcntl.h>
#include <windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

namespace {
void openFileByHandle(QFile* file) {
    bool openedByHandle = false;

#ifdef Q_OS_WIN
    // Enable full sharing like KLOGG
    DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    int accessRights = GENERIC_READ;
    DWORD creationDisp = OPEN_EXISTING;

    // Create the file handle
    SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
    HANDLE fileHandle = CreateFileW(
        (const wchar_t*)file->fileName().utf16(), 
        accessRights, 
        shareMode,
        &securityAtts, 
        creationDisp, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL
    );

    if (fileHandle != INVALID_HANDLE_VALUE) {
        qDebug() << "Got native file handle" << (intptr_t)fileHandle;
        // Convert the HANDLE to an fd and pass it to QFile's foreign-open
        int fd = _open_osfhandle((intptr_t)fileHandle, _O_RDONLY);
        qDebug() << "Got fd" << fd;
        if (fd != -1) {
            openedByHandle = file->open(fd, QIODevice::ReadOnly, QFile::AutoCloseHandle);
        } else {
            qWarning() << "Failed to open file by handle" << file->fileName();
            ::CloseHandle(fileHandle);
        }
    } else {
        qWarning() << "Failed to open file by handle" << file->fileName();
    }
#endif

    if (!openedByHandle) {
        file->open(QIODevice::ReadOnly);
    }
    qDebug() << "QFile opened";
}
} // namespace

FileHolder::FileHolder(bool keepClosed)
    : keep_closed_(keepClosed), counter_(0) {
    qDebug() << "Created file holder" << reinterpret_cast<void*>(this);
}

FileHolder::~FileHolder() {
    qDebug() << "Destroy file holder" << reinterpret_cast<void*>(this) << "for" << file_name_;
}

void FileHolder::open(const QString& fileName) {
    QMutexLocker locker(&file_mutex_);
    file_name_ = fileName;

    qDebug() << "Open file" << file_name_ << "keep closed" << keep_closed_;

    if (!keep_closed_) {
        counter_ = 1;
        reOpenFile();
    }
}

void FileHolder::reOpenFile() {
    qDebug() << "Reopen" << file_name_;

    auto reopened = std::make_unique<QFile>(file_name_);
    if (QFileInfo(file_name_).isReadable()) {
        openFileByHandle(reopened.get());
    }

    QMutexLocker locker(&file_mutex_);
    attached_file_ = std::move(reopened);
}

qint64 FileHolder::size() {
    QMutexLocker locker(&file_mutex_);
    return attached_file_ ? attached_file_->size() : 0;
}

bool FileHolder::isOpen() {
    QMutexLocker locker(&file_mutex_);
    return attached_file_ ? attached_file_->openMode() != QIODevice::NotOpen : false;
}

QFile* FileHolder::getFile() {
    return attached_file_.get();
}

void FileHolder::attachReader() {
    QMutexLocker locker(&file_mutex_);

    if (keep_closed_ && counter_ == 0) {
        qDebug() << "First reader opened for" << file_name_;
        reOpenFile();
    }

    counter_++;
    qDebug() << "Has" << counter_ << "readers for" << file_name_;
}

void FileHolder::detachReader() {
    QMutexLocker locker(&file_mutex_);
    if (counter_ > 0) {
        counter_--;
    }

    if (keep_closed_ && counter_ == 0) {
        attached_file_->close();
        qDebug() << "Last reader closed for" << file_name_;
    }
}

void FileHolder::lock() {
    file_mutex_.lock();
}

void FileHolder::unlock() {
    file_mutex_.unlock();
} 