#ifndef LIBRARYWORKER_H
#define LIBRARYWORKER_H

#include <QtCore>
#include <QtSql>

#include "phxpaths.h"
#include "metadatadatabase.h"
#include "platform.h"
#include "platforms.h"
#include "systemdatabase.h"
#include "logging.h"

namespace Library {

    struct GameData {
        qreal importProgress;
        QString system;
        QString timePlayed;
        QString title;
        QString filePath;
        QString sha1;

        QString artworkUrl;
        QString goodToolsCode;
        QString region;
        QString developer;
        QString releaseDate;
        QString genre;
        QString description;

        bool updated = false;
        qint64 fileID;
    };

    struct HeaderData {
        QString result;
        QString systemIndex;
        qint64 seekPosition;
        qint64 byteLength;
    };

    struct CueData {
        QString system;
        QString sha1;
    };

    class LibraryWorker : public QObject {
            Q_OBJECT
        public:

            explicit LibraryWorker( QObject *parent = 0 );
            ~LibraryWorker();

            bool insertCancelled();
            bool insertPaused();
            bool isRunning();
            bool resumeQuitScan();

            QString resumeInsertID();
            QString resumeDirectory();

            void setResumeInsertID( const QString id );
            void setResumeDirectory( const QString directory );
            void setResumeQuitScan( const bool resume );

        signals:
            void started();
            void finished();

            void insertGameData( const GameData gameData );
            void processFileInfo( QFileInfo fileInfo );
            void progressChanged( qreal value );


        public slots:
            void setInsertCancelled( const bool cancelled );
            void setInsertPaused( const bool paused );

            void prepareMetadata( GameData &gameData );
            void findGameFiles( const QString localUrl );

            void eventLoopStarted();

            void handleDraggedUrls( QList<QUrl> urls );
            void handleDroppedUrls();
            void handleContainsDrag( const bool contains );


        private slots:
            void prepareGameData( QQueue<QFileInfo> &queue );

        private:
            bool mInsertCancelled;
            bool mInsertPaused;
            QMutex mMutex;
            QStringList mFileFilters;
            QQueue<QFileInfo> mFileInfoQueue;
            bool mRunning;
            bool mContainsDrag;
            bool qmlResumeQuitScan;

            QList<QUrl> mDraggedUrls;

            QString mResumeInsertID;
            QString mResumeDirectory;

            // Setters
            void setIsRunning( const bool running );

            // Helper Functions
            void checkHeaderOffsets( const QFileInfo &fileInfo, Platform::Platforms &platform );
            QStringList getCueFileInfo( const QFileInfo &fileInfo );
            QString getCheckSum( const QString &filePath );
            bool checkForBios( const QString &filePath, const QString &checkSum, QSqlQuery &query );

            void cacheBiosFile( const QString &filePath, const QString &biosName );
            QStringList getAvailableSystems( const QString &extension, QSqlQuery &query );
            QList<HeaderData> getPossibleHeaders( const QStringList &possibleSystems, QSqlQuery &query );
            QString getRealSystem( const QList<HeaderData> &possibleHeaders, const QString &gameFilePath, QSqlQuery &query );
            CueData getCueData( const QStringList &possibleSystems, const QFileInfo &fileInfo, QSqlQuery &query );

    };

}


#endif // LIBRARYWORKER_H
