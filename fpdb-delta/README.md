# Delta Manager
## Concept
`fpdb-delta` contains all building blocks for the Delta Manager module for the HTAP application.
It is essentially a cache for the incoming deltas. It will store the deltas in the cache
for both foreground merge and background merge to fetch. Foreground merge will only read from
the cache, while background merge will read and delete the entry from cache.

Each table will have one cache for all the partitions in the table.
For example, if table `lineorder` is partitioned to 100 partitions. Then,
there will be one cache with 100 possible cache keys in the cache of table `lineorder`.

`DeltaCacheKey` is essentially a wrapper class for `Partition`.
`DeltaCacheData` consists of the actual delta (`TupleSet`) and the timestamp when it's generated from binlog.

## Cache Operations
`DeltaCache::store`
- Stores the key-data pair into the cache. If the key exists, append to the slot as a different version

`DeltaCache::load`
- Retrieve all the versions of the deltas `DeltaCacheData` as a vector, given the key (or `Partition`)

`DeltaCache::remove`
- Remove all the versions of the cache entry given the underlying key 