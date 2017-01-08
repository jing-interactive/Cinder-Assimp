#include "SurfacePool.h"

#include "cinder/ImageIo.h"

using namespace ci;
using namespace model;

const std::shared_ptr<Surface>& SurfacePool::loadSurface( const fs::path& filepath )
{
	if(  mSurfaces.count( filepath ) ) {
		return mSurfaces.at( filepath );
	} else {
		std::shared_ptr<Surface> texture( new Surface( loadImage( filepath ) ) );
		mSurfaces.emplace( filepath, texture );
		return mSurfaces.at( filepath );
	}
}