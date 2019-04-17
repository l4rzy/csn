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

## Demo
Since this is a library, it's neccessary to build a demo program to test
```sh
$ make
$ export LD_LIBRARY_PATH=./build
$ ./demo "shallow"
```
The result should look like this
![demo](test/demo.png)

## Run tests
```sh
$ ninja test
```

## Licence
MIT

## Author(s)
* l4rzy (l4.foss@gmail.com)
