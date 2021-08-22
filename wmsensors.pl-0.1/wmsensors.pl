#!/usr/bin/perl -w
# wmsensors.pl-0.1 by Karsten Eiser <k.eiser@web.de> ©2003
# Perl-program which monitors the temperature and voltages of
# CPU and motherboard and also shows the Fan-speed.
# Runs as a dock app for Window Maker.
# Needs a proper configured installation of lmsensors.
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this software; if not, write to the Free
# Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

use strict;
use Wharf::JDockApp;
use Getopt::Long;

#___________________ setup ________________________________#

my $help = 0;
my $seconds = 10;
GetOptions( 
    "delay=i" => \$seconds,
	"help"    => \$help
) or exit &usage;

#_________________________ the program ____________________#

exit &usage if $help == 1;

my $display = 1;
SetSetup(  sub { SetDelay($seconds) } );
SetUpdate( \&update );
SetExpose( \&update );
# Switch between the kind of the displayed information
SetButton( sub { $display++; $display = 1 if $display > 2; &update; } );

StartApp;

#_____________________________________ subs _______________#

sub usage {
print <<HELP;
    Usage: $0 --delay [seconds]
           $0 --help	shows this message
    whithout the option "--delay" the default-value
    of 10 seconds will be used.
HELP
}

sub update {
	my ($color,
		$cpu_f, $cpu_m, # CPU-Fan: f = Fan, m = min
		$ps_f,  $ps_m,  # P/S Fan:
		$sys_t, $sys_l, $sys_h, # SYS Temp: t = Temp, l = limit, h = hysteresis
		$cpu_t, $cpu_l, $cpu_h, # CPU Temp:
		$sbr_t, $sbr_l, $sbr_h, # SBr Temp:
		$core, $core_min, $core_max,          # CPU core:
		$twoV5, $twoV5_min, $twoV5_max,    # +2.5V:
		$ioV, $ioV_min, $ioV_max,             # I/O:
		$fiveV, $fiveV_min, $fiveV_max,    # +5V:
		$twelfV, $twelfV_min, $twelfV_max, # +12V:
		);
	my @sensors = `sensors`;
	my $TEMP_EXPR = '.*Temp:\s+.(\d+\.\d+).*limit =\s+.(\d+).*hysteresis =\s+.(\d+)';
	my $FAN_EXPR = '.*Fan:\s+(\d+).*min\s+=\s+(\d+)';
	my $VOLTAGE_EXPR = '.*:\s+.(\d+\.\d+).*min\s+=\s+.(\d+\.\d+).*max\s+=\s+.(\d+\.\d+)';
	foreach (@sensors) {
		if ($display == 1) {
			if (/^CPU Fan/i) {
				/$FAN_EXPR/;
				($cpu_f, $cpu_m) = ($1, $2);
				next;
			}
			if (/^P\/S Fan/i) {
				/$FAN_EXPR/;
				($ps_f, $ps_m) = ($1, $2);
				next;
			}
			if (/^SYS Temp/i) {
				/$TEMP_EXPR/;
				($sys_t, $sys_l, $sys_h) = ($1, $2, $3);
				next;
			}
			if (/^CPU Temp/i) {
				/$TEMP_EXPR/;
				($cpu_t, $cpu_l, $cpu_h) = ($1, $2, $3);
				next;
			}
			if (/^SBr Temp/i) {
				/$TEMP_EXPR/;
				($sbr_t, $sbr_l, $sbr_h) = ($1, $2, $3);
				next;
			}
		} elsif ($display == 2) {
			if (/^CPU core/i) {
				/$VOLTAGE_EXPR/;
				($core, $core_min, $core_max) = ($1, $2, $3);
				next;
			}
			if (/^\+2\.5V/i) {
				/$VOLTAGE_EXPR/;
				($twoV5, $twoV5_min, $twoV5_max) = ($1, $2, $3);
				next;
			}
			if (/^I\/O/i) {
				/$VOLTAGE_EXPR/;
				($ioV, $ioV_min, $ioV_max) = ($1, $2, $3);
				next;
			}
			if (/^\+5V/i) {
				/$VOLTAGE_EXPR/;
				($fiveV, $fiveV_min, $fiveV_max) = ($1, $2, $3);
				next;
			}
			if (/^\+12V/i) {
				/$VOLTAGE_EXPR/;
				($twelfV, $twelfV_min, $twelfV_max) = ($1, $2, $3);
				next;
			}
		}
	}
   	ClearWindow;

	# First-view: Fan-speeds and temperatures
	if ($display == 1) {
	   	jpprint(0, 0, YELLOW, "Fan & Temp");
	
		$color = &color1($cpu_m, $cpu_m, $cpu_f);
   		jpprint(0, 1, $color, "CPU F");
   		jpprint(6, 1, $color, "$cpu_f");
   	
		$color = &color1($ps_m, $ps_m, $ps_f);
   		jpprint(0, 2, $color, "P/S F");
   		jpprint(6, 2, $color, "$ps_f");
   	
		$color = &color1($sys_t, $sys_l, $sys_h);
   		jpprint(0, 3, $color, "SYS T");
   		jpprint(6, 3, $color, "$sys_t");
   	
		$color = &color1($cpu_t, $cpu_l, $cpu_h);
		jpprint(0, 4, $color, "CPU T");
		jpprint(6, 4, $color, "$cpu_t");
   	
		$color = &color1($sbr_t, $sbr_l, $sbr_h);
		jpprint(0, 5, $color, "SBr T");
		jpprint(6, 5, $color, "$sbr_t");
	# Second-view: Monitor the voltages
	} elsif ($display == 2) {
	   	jpprint(0, 0, YELLOW, "Voltages");
		
		$color = &color2($core, $core_min, $core_max);
   		jpprint(0, 1, $color, "core");
   		jpprint(6, 1, $color, "$core");
   	
		$color = &color2($twoV5, $twoV5_min, $twoV5_max);
   		jpprint(0, 2, $color, "+2.5V");
   		jpprint(6, 2, $color, "$twoV5");
   	
		$color = &color2($ioV, $ioV_min, $ioV_max);
   		jpprint(0, 3, $color, "I/O");
   		jpprint(6, 3, $color, "$ioV");
   	
		$color = &color2($fiveV, $fiveV_min, $fiveV_max);
		jpprint(0, 4, $color, "+5V");
		jpprint(6, 4, $color, "$fiveV");
   	
		$color = &color2($twelfV, $twelfV_min, $twelfV_max);
		jpprint(0, 5, $color, "+12V");
		jpprint(6, 5, $color, "$twelfV");
	}
}

sub color1 {
	my ($t, $l, $h) = @_; # Temperatur, limit, hysteresis
	my $col;
	if ($t < $h) {
		$col = GREEN; # It`s okay
	} elsif ($t >= $h and $t < $l) {
		$col = ORANGE; # The value is near the limit
	} else {
		$col = RED; # Fire on your motherboard!! :-O
	}
	return($col);
}

sub color2 {
	my ($v, $l, $h) = @_; # Volt, lowest_voltage, highest_voltage
	my $col;
	if ($v < $l) {
		$col = ORANGE; # To low voltage
	} elsif ($v >= $l and $v <= $h) {
		$col = GREEN; # Voltage is okay
	} else {
		$col = RED; # To high voltage
	}
	return($col);
}
