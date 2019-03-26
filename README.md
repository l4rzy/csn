# Description
* A WIP C library for fetching data from [chiasenhac.com](http://chiasenhac.com)
* Built upon [libcurl](https://curl.haxx.se) and [libtidy](https://www.html-tidy.org/)

## Features
* Searching for audio/video/album
* Fetching hot/trending song list
* Get download link for audio/video/album

## APIs that work so far
- [x] csn_search
- [ ] csn_fetch_hot
- [ ] csn_fetch_song_info_url
- [ ] csn_fetch_album_info_url
- [ ] csn_fetch_song_info
- [ ] csn_fetch_album_info
- [ ] csn_batch_fetch_song_info

## Build
```sh
$ meson build
$ cd build && ninja
```

## Run tests
```sh
$ make
$ sh ./run_test.sh
```

## Author(s)
* l4rzy (l4.foss@gmail.com)
