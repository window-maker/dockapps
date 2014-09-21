#!/usr/bin/perl

# Update http://windowmaker.org/dockapps from git
#
# Copyright 2014 Window Maker Developers Team
#                <wmaker-dev@lists.windowmaker.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# DESCRIPTION
#
# After every commit to the dockapps git repository, run this script to
# update dockapps/dockapps.db, the plain text config file that stores all
# of the information for the dockapps section of windowmaker.org.
#
# The script uses git to determine the version numbers and tarball download
# urls.  Everything else is pulled in from dockapps.db.in, which is
# manually updated.
#
# The format of each dockapp entry in dockapps.db.in is:
#
# [dockapp name]
# image = (file name of image; in double quotes and comma-separated if more
#   than one image)
# description = (description, possibly taken from archive.org copy of
#   dockapps.windowmaker.org; in double quotes)
# url = (use archive.org copy if original homepage no longer exists)
# dockapps = (currently unused; id number from dockapps.windowmaker.org)
# category = (category from dockapps.windowmaker.org)
#
# After generating a new dockapps.db, submit a patch for the whome git repo
# to wmaker-dev@lists.windowmaker.org.

use warnings;
use strict;
use Git::Repository;
use POSIX;
use Debian::Dpkg::Version;

open DB, "dockapps.db.in" or die $!;
my $db = do { local $/; <DB> };
close DB;

my $r = Git::Repository->new();
my @tags = $r->run("tag");
my %dockapps;

# If any earlier versions of a dockapp had an alternate name, e.g. a name change
# or a fork which has since been blessed, add it to this hash as 'alt' =>
# 'main'.  The alternate should still have its own entry in dockapps.db.in.
my %alts = (
	'wmacpi-ng' => 'wmacpi'
    );

foreach my $tag (@tags) {
	$tag =~ /([\w\-+.]+)-([\w.]+)/;
	my $dockapp = $1;
	my $version = $2;
	my $ls = $r->run("ls-tree", $tag, $dockapp);
#for older tags from before directory renaming
	if (!$ls) {
		$ls = $r->run("ls-tree", $tag, $tag);
	}
#for wmfemon_1
	if (!$ls) {
		$ls = $r->run("ls-tree", $tag, "$dockapp" . "_$version");
	}
#for alts
	if (!$ls) {
		$ls = $r->run("ls-tree", $tag, $alts{$dockapp});
	}
	my $sha1 = (split(/\s/, $ls))[2];
	$dockapps{$dockapp}{$version} = $sha1;
}

foreach my $dockapp (keys %dockapps) {
	if (grep {$_ eq $dockapp} keys %alts) {
		next;
	}
	my $latest_version = (sort by_version keys $dockapps{$dockapp})[-1];
	if ($r->run("diff", "$dockapp-$latest_version", "HEAD", "--", $dockapp)) {
		my $commit = $r->run("log", "-1",
				  "--pretty=format:%H", "--", $dockapp);
		my $date = strftime("%Y%m%d", localtime($r->run("log", "-1",
				  "--pretty=format:%ct", "--", $dockapp)));
#throw out dockapps whose last commit was stripping version names from dirs
		unless ($commit eq "eea379d83350ced6166099ebc8f41ff4e3fa1f42") {
			my $ls = $r->run("ls-tree", $commit, $dockapp);
			if (!$ls) {
				$ls = $r->run("ls-tree", $commit,
					      "$dockapp-$latest_version");
			}
			my $sha1 = (split(/\s/, $ls))[2];
			$dockapps{$dockapp}{"$latest_version+$date"} = $sha1;
		}
	}
}


foreach my $dockapp (keys %dockapps) {
	my $versions = "";
	foreach my $version (reverse sort by_version keys $dockapps{$dockapp}) {
		$versions .= "version-$version = " .
		    $dockapps{$dockapp}{$version} . "\n";
	}
	my $search = quotemeta "[$dockapp]\n";
	$db =~ s/$search/[$dockapp]\n$versions/;
}

open DB, ">dockapps.db" or die $!;
print DB $db;
close DB;

sub by_version {
	(my $left = $a) =~ s/(a|b|rc)/~$1/;
	(my $right = $b) =~ s/(a|b|rc)/~$1/;
	Debian::Dpkg::Version->new($left) <=>
		Debian::Dpkg::Version->new($right);
}
