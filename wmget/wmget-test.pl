#!/usr/bin/perl -w
use strict;

use IO::Handle;
use IO::Socket;

&run_server ();

sub run_server {
    local $SIG{PIPE} = 'IGNORE';

    my $servsock = new IO::Socket (
        Domain => AF_INET,
        Type => SOCK_STREAM,
        Proto => "tcp",
        Reuse => 1,
        Listen => 1,
        LocalPort => 8000,
    ) or die "new IO::Socket: $!";

    print "to test, enter wmget http://localhost:8000/...\n";

    while (my $client = $servsock->accept) {
        $client->autoflush (1);

        while (<$client>) {
            print STDERR "> $_";
            last if not /\S/;
        }

        print STDERR "headers done.  sending data...\n";

        print $client "HTTP/1.0 200 Ok, here you go...\r\n";
        print $client "Content-Type: text/plain\r\n";
        print $client "Content-Length: 1000\r\n\r\n";

        # generate bogus data, and do it slowly....
        for (1..10) {
            print $client "x" x 99, "\n";
            sleep 1;
        }
    }
}


