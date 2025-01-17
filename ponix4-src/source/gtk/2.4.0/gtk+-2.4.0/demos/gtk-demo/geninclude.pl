#!/usr/bin/perl -w

print <<EOT;
typedef	GtkWidget *(*GDoDemoFunc) (GtkWidget *do_widget);

typedef struct _Demo Demo;

struct _Demo 
{
  gchar *title;
  gchar *filename;
  GDoDemoFunc func;
  Demo *children;
};

EOT

for $file (@ARGV) {
    my %demo;
    
    ($basename = $file) =~ s/\.c$//;

    open INFO_FILE, $file or die "Cannot open '$file'\n";
    $title = <INFO_FILE>;
    $title =~ s@^\s*/\*\s*@@;
    $title =~ s@\s*$@@;

    close INFO_FILE;

    print "GtkWidget *do_$basename (GtkWidget *do_widget);\n";

    push @demos, {"title" => $title, "file" => $file,
		  "func"  => "do_$basename"};
}

# generate a list of 'parent names'
foreach $href (@demos) {
    if ($href->{"title"} =~ m|^([\w\s]+)/[\w\s]+$|) {
	my $parent_name = $1;
	my $do_next = 0;

	# parent detected
	if (defined @parents) {
	    foreach $foo (@parents) {
		if ($foo eq $parent_name) {
		    $do_next = 1;
		}
	    }
	    
	    if ($do_next) {
		next;
	    }
	}

	push @parents, $parent_name;

	$tmp = (defined @child_arrays)?($#child_arrays + 1):0;
	push @child_arrays, "child$tmp";

	push @demos, {"title" => $parent_name, "file" => "NULL",
		      "func" => "NULL"};
    }
}

if (defined @parents) {
    $i = 0;
    for ($i = 0; $i <= $#parents; $i++) {
	$first = 1;
	
	print "\nDemo ", $child_arrays[$i], "[] = {\n";
	
	$j = 0;
	for ($j = 0; $j <= $#demos; $j++) {
	    $href = $demos[$j];
	    
	    if (!defined $demos[$j]) {
		next;
	    }
	    
	    if ($demos[$j]{"title"} =~ m|^$parents[$i]/([\w\s]+)$|) {
		if ($first) {
		    $first = 0;
		} else {
		    print ",\n";
		}
		
		print qq (  { "$1", "$demos[$j]{file}", $demos[$j]{func}, NULL });

		# hack ... ugly
		$demos[$j]{"title"} = "foo";
	    }
	}

	print ",\n";
	print qq (  { NULL } );
	print "\n};\n";
    }   
}

# sort @demos
@demos_old = @demos;

@demos = sort {
    $a->{"title"} cmp $b->{"title"};
} @demos_old;

# sort the child arrays
if (defined @child_arrays) {
    for ($i = 0; $i <= $#child_arrays; $i++) {
	@foo_old = @{$child_arrays[$i]};

	@{$child_arrays[$i]} = sort {
	    $a->{"title"} cmp $b->{"title"};
	} @foo_old;
    }
}

# toplevel
print "\nDemo testgtk_demos[] = {\n";

$first = 1;
foreach $href (@demos) {
    $handled = 0;

    # ugly evil hack
    if ($href->{title} eq "foo") {
	next;
    }

    if ($first) {
	$first = 0;
    } else {
	print ", \n";
    }

    if (defined @parents) {
	for ($i = 0; $i <= $#parents; $i++) {
	    if ($parents[$i] eq $href->{title}) {

		if ($href->{file} eq 'NULL') {
		    print qq (  { "$href->{title}", NULL, $href->{func}, $child_arrays[$i] });
		} else {
		    print qq (  { "$href->{title}", "$href->{file}", $href->{func}, $child_arrays[$i] });
		}
		
		$handled = 1;
		last;
	    }
	}
    }
    
    if ($handled) {
	next;
    }
    
    print qq (  { "$href->{title}", "$href->{file}", $href->{func}, NULL });
}

print ",\n";
print qq (  { NULL } );
print "\n};\n";

exit 0;
