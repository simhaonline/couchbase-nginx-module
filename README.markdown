# nginx-couchbase-module

Hi, this is couchbase module for nginx. Here is short build
instructions. More docs are coming.

Your test machine should have C compiler installed, as well as
autotools.

    mkdir couchbase-nginx-module
    cd couchbase-nginx-module
    repo init -u git://github.com/trondn/manifests.git -m nginx.xml
    repo sync
    make

# Important Note

This release depends on [fork of libcouchbase][1], therefore it **will
not work** with the client, you can download from binary repositories.
The most recent version of the forked library is **2.0.3nginx3** and
you can download it using this URL:
http://packages.couchbase.com/clients/c/libcouchbase-2.0.3nginx3.tar.gz

The changes from the fork are going to be integrated to upstream soon.

# Testing

There is way to run all tests against different versions of nginx, in
case you are using repo to checkout sources:

    cd module
    make check NGX_VERSIONS="v1.4.0 v1.3.10"

By default `make check` will run use `NGX_VERSIONS="v1.2.6 v1.4.0"`.

Note that until version [v1.3.10](https://github.com/nginx/nginx/commit/v1.3.10)
nginx has a bug which doesn't allow to add headers to 201 responses,
therefore corresponding tests are skipped in the testsuite for these
versions.

There is also make target for stress testing using Apache Benchmark:
`make stress`. It also honours `NGX_VERSIONS` and also depends on
several more variables. These are default values, pretty
self-explanatory:

    NGX_VERSIONS="v1.2.6 v1.4.0"
    AB_CONCURRENCY=10
    AB_REQUESTS=1000
    AB_NGX_CONFIG=etc/nginx.stress.conf
    AB_URI=http://localhost:8080/lcb?cmd=set&key=foo&val=bar

# Usage

This section describes step by step guide how to build nginx with
couchbase module. This version (0.3.1) is only tested with
nginx v1.3.7, v1.2.6 and v1.4.0, other versions might also work, more
broad coverage is subject of upcoming releases. Also it requires
modified libcouchbase library from `nginx` branch of my fork
[avsej/libcouchbase][1].

Create build directory and download all dependencies there:

    mkdir nginx-couchbase
    cd nginx-couchbase
    wget http://nginx.org/download/nginx-1.3.7.tar.gz
    wget http://packages.couchbase.com/clients/c/libcouchbase-2.0.3nginx3.tar.gz
    wget http://packages.couchbase.com/clients/c/nginx-couchbase-module-0.3.1.tar.gz
    for i in *.tar.gz; do tar xvf $i; done

Build and install everything into `/opt/nginx-couchbase` prefix:

    export PREFIX=/opt/nginx-couchbase

    cd libcouchbase-2.0.3nginx3
    ./configure --prefix=$PREFIX --enable-debug --disable-plugins --disable-tests --disable-couchbasemock
    make && sudo make install
    cd ..

    cd nginx-1.3.7
    export LIBCOUCHBASE_INCLUDE=$PREFIX/include
    export LIBCOUCHBASE_LIB=$PREFIX/lib
    ./configure --prefix=$PREFIX --with-debug --add-module=../nginx-couchbase-module-0.3.1
    make && sudo make install

Now you should install and configure Couchbase Server. Read more at
official site: http://www.couchbase.com/download. Make sure you have
configured bucket `default`.

Given all stuff installed, lets deploy the simplest config file,
which can be find in `etc/nginx.conf` of this repository. Here is
`server` section of this config:

    server {
        listen       8080;
        server_name  localhost;
        location /cb {
            set $couchbase_key $arg_key;
            set $couchbase_cmd $arg_cmd;
            set $couchbase_val $arg_val;
            couchbase_connect_timeout 2ms;
            couchbase_timeout 1.5ms;
            couchbase_pass localhost:8091,127.0.0.1:8091,localhost bucket=default;
        }
    }

To deploy and check config file validness, run this commands (assuming
your current directory is the root of this repository).

    sudo cp etc/nginx.conf /opt/nginx-couchbase/conf/nginx.conf
    sudo /opt/nginx-couchbase/sbin/nginx -t

It should report that config is valid and everything is ok. Lets start
the server and play with it using `curl`.

    sudo /opt/nginx-couchbase/sbin/nginx

Put key into the storage

    $ curl -v 'http://localhost:8080/cb?cmd=set&key=foo&val=bar'
    * About to connect() to localhost port 8080 (#0)
    *   Trying 127.0.0.1...
    * Connected to localhost (127.0.0.1) port 8080 (#0)
    > GET /cb?cmd=set&key=foo&val=bar HTTP/1.1
    > User-Agent: curl/7.29.0
    > Host: localhost:8080
    > Accept: */*
    >
    < HTTP/1.1 201 Created
    < Server: nginx/1.3.7
    < Date: Wed, 17 Apr 2013 23:47:06 GMT
    < Content-Length: 0
    < Connection: keep-alive
    < X-Couchbase-CAS: 16808439146359685120
    <
    * Connection #0 to host localhost left intact

Fetch document back

    $ curl -v  'http://localhost:8080/cb?cmd=get&key=foo'
    * About to connect() to localhost port 8080 (#0)
    *   Trying 127.0.0.1...
    * Connected to localhost (127.0.0.1) port 8080 (#0)
    > GET /cb?cmd=get&key=foo HTTP/1.1
    > User-Agent: curl/7.29.0
    > Host: localhost:8080
    > Accept: */*
    >
    < HTTP/1.1 200 OK
    < Server: nginx/1.3.7
    < Date: Wed, 17 Apr 2013 23:48:16 GMT
    < Content-Length: 3
    < Connection: keep-alive
    < X-Couchbase-CAS: 11078484393219129344
    <
    * Connection #0 to host localhost left intact

Delete document from the storage

    $ curl -v  'http://localhost:8080/cb?cmd=delete&key=foo'
    * About to connect() to localhost port 8080 (#0)
    *   Trying 127.0.0.1...
    * Connected to localhost (127.0.0.1) port 8080 (#0)
    > GET /cb?cmd=delete&key=foo HTTP/1.1
    > User-Agent: curl/7.29.0
    > Host: localhost:8080
    > Accept: */*
    >
    < HTTP/1.1 200 OK
    < Server: nginx/1.3.7
    < Date: Wed, 17 Apr 2013 23:48:49 GMT
    < Content-Length: 0
    < Connection: keep-alive
    < X-Couchbase-CAS: 11150541987257057280
    <
    * Connection #0 to host localhost left intact

If you will skip `$couchbase_key`, `$couchbase_cmd`, `$couchbase_val`
declarations it will use URL, HTTP method and HTTP body
correspondingly.

[1]: https://github.com/avsej/libcouchbase/tree/nginx
