project(fpdb-delta VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)

#-----------------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-delta STATIC
        # Cache Key
        src/DeltaCacheKey.cpp include/fpdb/delta/DeltaCacheKey.h
        # Cache Value (Data)
        src/DeltaCacheData.cpp include/fpdb/delta/DeltaCacheData.h
        # Cache Class
        src/DeltaCache.cpp include/fpdb/delta/DeltaCache.h
        src/KeepMapCacheKey.cpp include/fpdb/delta/KeepMapCacheKey.h
        src/KeepMapCacheData.cpp include/fpdb/delta/KeepMapCacheData.h
        src/KeepMapCache.cpp include/fpdb/delta/KeepMapCache.h
        include/fpdb/delta/DeltaCacheMetadata.h
        src/StableBmCacheKey.cpp include/fpdb/delta/StableBmCacheKey.h
        src/StableBmCacheData.cpp include/fpdb/delta/StableBmCacheData.h
        src/StableBmCache.cpp include/fpdb/delta/StableBmCache.h)

target_include_directories(fpdb-delta PUBLIC include)

target_link_libraries(fpdb-delta PUBLIC fpdb-catalogue)
target_link_libraries(fpdb-delta PUBLIC fpdb-hermesserver)

