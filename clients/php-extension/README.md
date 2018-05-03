Snowflake PHP Client

This extension provides an API for communicating with Snowflaked.

# Building

    phpize
    ./configure
    make
    sudo make install

Once built and installed, be sure to add 'snowflake.so' to the php.ini extensions configuration.

    extension=snowflake.so

# Use

This extension exposes one class, 'Snowflake'.

    $sf = new Snowflake();
    $sf->connect('127.0.0.1', 8008);
    $id = $sf->get();
    $stats = $sf->info();


### TODO
Real class docs
