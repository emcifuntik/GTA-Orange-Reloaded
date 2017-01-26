#!/usr/bin/perl

my %extfns = ();
my %decls = ();
my $ver = '';
my $glexth = '/usr/include/GL/glext.h';

for (my $i = 0; $i <= $#ARGV; $i++) {
  $ver = 'ES' if ($ARGV[$i] eq '-es');
  if ($ARGV[$i] eq '-h') {
    $glexth = $ARGV[$i+1];
  }
}

open(EXTS, "<Extensions$ver.txt");
while (<EXTS>) {
  if (/(gl\w+)(?:\s+([A-Z]+\s*)*)?/) {
    $extfns{$1} = [split(/\s+/, $2)];
  }
}
close(EXTS);

unless (-f $glexth) {
  $glexth = "../../../3rdParty/glext/glext.h";
}
open(GLEXT, "<$glexth");
while (<GLEXT>) {
  if (/(?:GLAPI\s+)?([\w*]+)\s+(?:(?:GL_)?APIENTRY\s+)?(gl\w+) *\((.*)\)/) {
    if ($extfns{$2}) {
      my ($ret,$name,$proto) = ($1,$2,$3);
      $proto =~ s/(\w+)(?: +\w+)/\1/g;
      $decls{$name} = [$ret,split(/\s*,\s*/, $proto)];
    }
  }
};
close(GLEXT);

open(OUTH, ">GL${ver}_Extensions.h");
open(OUTM, ">GL${ver}_ExtensionMacros.h");
open(OUTC, ">GL${ver}_Extensions.cpp");

print OUTH <<"EOF";

namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
EOF

print OUTC <<"EOF";

#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

bool Extensions::Init()
{
    bool result = 1;
EOF

print OUTM "#include \"Render/GL/GL_HAL.h\"\n\n";

my $maxlen = 0;
foreach my $fn (keys %extfns) {
  $maxlen = length($fn) if (length($fn) > $maxlen);
}
$maxlen += 7;

my @calls = ();

foreach my $fn (keys %extfns) {
  die "function $fn not defined" unless $decls{$fn};
  my @args = (@{$decls{$fn}});
  my $ret = shift @args;
  my @pargs = ();
  my @params = ();

  if ("@args" ne 'void') {
    for (my $i = 0; $i <= $#args; $i++) {
      push @pargs, $args[$i] . " a$i";
      push @params, "a$i";
    }
  }

  my $pfn = uc("PFN${fn}PROC");
  printf OUTH "    %-${maxlen}s p_$fn;\n", $pfn;
  print OUTM "#define $fn GetHAL()->$fn\n";

  my $c = sprintf "    %s %s(%s)\n    {\n        ", $ret, $fn, join(', ', @pargs);
  $c .= 'return ' if ($ret ne 'void');
  $c .= "p_$fn(".join(', ', @params).");\n    }\n\n";

  push @calls, $c;

  print OUTC "    p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"$fn\");\n";

  if (!($fn =~ /(?:EXT|ARB|OES)$/)) {
print OUTC <<"EOF";
    if (!p_$fn)
    {
         p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"${fn}ARB\");
         if (!p_$fn)
         {
              p_$fn = ($pfn) SF_GL_RUNTIME_LINK(\"${fn}EXT\");
              if (!p_$fn)
                  result = 0;
         }
    }

EOF
}

  print OUTC "\n";
}

print OUTH "\npublic:\n    bool Init();\n\n";

foreach (@calls) {
  print OUTH;
}

print OUTH "};\n\n}}}\n";
print OUTC "    return result;\n}\n\n}}}\n";
close(OUTH);
close(OUTC);
close(OUTM);
