#ifndef INDEXER_H
#define INDEXER_H

#include <QtCore>
#include <AddMessage.h>
#include "Rdm.h"

typedef QHash<RTags::Location, Rdm::CursorInfo> SymbolHash;
typedef QHash<QByteArray, QSet<RTags::Location> > SymbolNameHash;
typedef QHash<Path, QSet<Path> > DependencyHash;
typedef QPair<QByteArray, quint64> WatchedPair;
typedef QHash<Path, QSet<WatchedPair> > WatchedHash;
struct FileInformation {
    FileInformation() : lastTouched(0) {}
    time_t lastTouched;
    QList<QByteArray> compileArgs;
};
static inline QDataStream &operator<<(QDataStream &ds, const FileInformation &ci)
{
    ds << static_cast<quint64>(ci.lastTouched) << ci.compileArgs;
    return ds;
}

static inline QDataStream &operator>>(QDataStream &ds, FileInformation &ci)
{
    quint64 lastTouched;
    ds >> lastTouched;
    ci.lastTouched = static_cast<time_t>(lastTouched);
    ds >> ci.compileArgs;
    return ds;
}

typedef QHash<Path, FileInformation> InformationHash;

class IndexerJob;
class IndexerSyncer;
class Indexer : public QObject
{
    Q_OBJECT;
public:

    Indexer(const QByteArray& path, QObject* parent = 0);
    ~Indexer();

    int index(const QByteArray& input, const QList<QByteArray>& arguments);

    void setDefaultArgs(const QList<QByteArray> &args);
    QList<QByteArray> defaultArgs() const { return mDefaultArgs; }
    void setPchDependencies(const Path &pchHeader, const QSet<Path> &deps);
    QSet<Path> pchDependencies(const Path &pchHeader) const;
    void poll();
protected:
    void timerEvent(QTimerEvent *e);
    void customEvent(QEvent* event);

signals:
    void indexingDone(int id);

private slots:
    void onJobDone(int id, const QByteArray& input);
    void onDirectoryChanged(const QString& path);
private:
    void commitDependencies(const DependencyHash& deps, bool sync);
    void initWatcher();
    void init();

    QList<QByteArray> mDefaultArgs;
    mutable QReadWriteLock mPchDependenciesLock;
    QHash<Path, QSet<Path> > mPchDependencies;
    int mJobCounter;

    QMutex mMutex;
    QWaitCondition mCondition;
    QSet<QByteArray> mIndexing;
    QSet<QByteArray> mPchHeaderError;

    QByteArray mPath;
    int mLastJobId;
    QHash<int, IndexerJob*> mJobs;

    IndexerSyncer* mSyncer;

    bool mTimerRunning;
    QElapsedTimer mTimer;

    QFileSystemWatcher mWatcher;
    DependencyHash mDependencies;
    QMutex mWatchedMutex;
    WatchedHash mWatched;
    QBasicTimer mPollTimer;

    friend class IndexerJob;
};

#endif
