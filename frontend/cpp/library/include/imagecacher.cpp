#include "imagecacher.h"
#include "logging.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QFile>
#include <QFileInfo>

using namespace Library;

ImageCacher::ImageCacher( QObject *parent )
    : QObject( parent ),
      mNetworkManager( this ) {

    connect( &mNetworkManager, &QNetworkAccessManager::finished, this, &ImageCacher::handleRequest );

}

void ImageCacher::cache() {

    static const auto creation = ImageCacher::createCachePath();
    Q_UNUSED( creation );

    if( imageUrl().isEmpty() ) {
        return;
    }

    auto urlString = imageUrl().toString();
    mImageType = QFileInfo( urlString ).suffix();

    auto cachedFile = cacheDirectory + QDir::separator() + identifier() + "." + mImageType;

    if( !QFile::exists( cachedFile ) && imageUrl().isValid() ) {

        QNetworkRequest request( imageUrl() );
        auto *reply = mNetworkManager.get( request );

        reply->setProperty( "cachedAbsoluteFilePath", cachedFile );

        connect( reply, SIGNAL( error( QNetworkReply::NetworkError ) )
                 , this, SLOT( handleRequestError( QNetworkReply::NetworkError ) ) );

        connect( reply, &QNetworkReply::downloadProgress, this, &ImageCacher::handleRequestProgress );
        connect( reply, &QNetworkReply::sslErrors, this, &ImageCacher::handleSSLErrors );

        return;
    }

    setCachedUrl( QUrl( qmlFilePrefix + cachedFile ) );

}

QUrl ImageCacher::cachedUrl() const {
    return qmlCachedUrl;
}

QUrl ImageCacher::imageUrl() const {
    return qmlImageUrl;
}

QString ImageCacher::identifier() const {
    return qmlIdentifier;
}

void ImageCacher::setIdentifier( const QString id ) {
    qmlIdentifier = id;
    emit identifierChanged();
}

void ImageCacher::setImageUrl( const QUrl url ) {
    qmlImageUrl = url;
    emit imageUrlChanged();
}

void ImageCacher::setCachedUrl( const QUrl url ) {
    qmlCachedUrl = url;
    emit cachedUrlChanged();
}

bool ImageCacher::createCachePath() {
    QDir cacheDir( cacheDirectory );

    if( !cacheDir.exists() ) {
        return cacheDir.mkpath( cacheDir.absolutePath() );
    }

    return true;
}

void ImageCacher::handleRequest( QNetworkReply *reply ) {

    if( !reply->error() ) {
        auto imageBytes = reply->readAll();

        QFile file( cacheDirectory + QDir::separator() + identifier() + "." + mImageType );

        if( file.open( QIODevice::WriteOnly ) ) {

            if( file.write( std::move( imageBytes ) ) == -1 ) {
                qCWarning( phxLibrary ) << "Couldn't cache " << identifier() << "to" << cacheDirectory;
            }

            else {
                setCachedUrl( std::move( QUrl( qmlFilePrefix + reply->property( "cachedAbsoluteFilePath" ).toString() ) ) );
            }

            file.close();

        }

    }

    reply->deleteLater();
}

void ImageCacher::handleRequestError( QNetworkReply::NetworkError error ) {
    switch( error ) {
        default:
            qCDebug( phxLibrary ) << "QNetworkReply Error: " << error;
            break;
    }
}

void ImageCacher::handleRequestProgress( qint64 bytesRecieved, qint64 bytesTotal ) {
    Q_UNUSED( bytesRecieved );
    Q_UNUSED( bytesTotal );
    //qCDebug( phxLibrary ) << "Downloading Image: "
      //                    << ( bytesRecieved / static_cast<qreal>( bytesTotal ) ) * 100.0;
}

void ImageCacher::handleSSLErrors( const QList<QSslError> &errors ) {
    Q_UNUSED( errors );
}
