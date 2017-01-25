
package Shaders::D3D9;

@ISA = qw/Shaders/;

sub PlatformNames { qw/D3D9 X360/ }

sub new { bless {} }

sub Caps { 'Ureg' }

sub GetSem
{
  my ($vt,$vn,$vs) = ($_[0]->{type},$_[0]->{name},$_[0]->{sem});
  if ($vs eq '') {
    if ($vn =~ /^a?tc([0-9]*)/) { $vs = 'TEXCOORD'.(0+$1); }
    elsif ($vn eq 'color') { $vs = 'COLOR0'; }
    elsif ($vn eq 'factor') { $vs = 'COLOR1'; }
    elsif ($vn =~ /^v?pos/) { $vs = 'POSITION'; }
  }
  return $vs;
}

sub Param
{
  $_ = $_[1];

  if ($_->{type} =~ /^sampler(.*)$/) {
    my $s = "sampler $_->{name} : register(s\$texn)".($_[2]?';':'');
	$_->{uofs} = $texn;
    return $s, 2, {qw/texn $$texn++/};
  }

  my ($vt,$vn,$vs) = ($_->{type},$_->{name},GetSem($_));
  $vt =~ s/vec/float/g;
  if (exists $_->{dim}) {
    $vn .= "[$_->{dim}]";
  }

  if ($_[0]->{stage} eq 'v' && $_->{vary} eq 'varying') { $vt = "out $vt"; }
  elsif ($_[0]->{stage} eq 'f' && $_->{vary} eq 'fragout') { $vt = "out $vt"; }
  my $s = "$vt $vn";
  $s.= " : $vs" if $vs;
  if ($_->{vary} eq 'uniform') {
    $s .= " : register(c\$unin)".($_[2]?';':'');
	return $s, 1, {unin=> '$$unin+='.Shaders::VarSize($_)};
  }
  else {
    $s .= ',' if $_[2];
	return $s, 0;
  }
}

sub BuiltinParams
{
  return ($Shaders::stage eq 'f' ? ({vary => 'fragout', type => 'float4', name => 'fcolor', sem => 'COLOR'}) : ());
}

sub Block
{
  my ($this, $stage, $argsr, $b) = @_;
  my %args = %{$argsr};

  $b = Shaders::BlockD3D($this, $stage, $argsr, $b);
  Shaders::ParamD3D($b);
#  $b =~ s/float([234]) *\($Shaders::subexpr\)/"float$1(".join(',',($2)x$1).")"/ge;

  return $b;
}

sub FileExt
{
  return 'hlsl';
}

sub Output
{
  my $this = $_[0];
  if ($_[1] eq 'b') {
    return [0, sub { return if Shaders::CheckOut(@_);
                     my $f = "$this->{dir}/$_[0].".$this->FileExt($stage);
                     open(OUT,">$f") || die $f;
                     print OUT $_[1];
                     close(OUT);
                     my $prof = 'ps_3_0';
                     $prof = 'vs_3_0' if ($_[0] =~ /^V/);
                     system("fxc.exe /nologo /E main /T $prof /Fh $f.h /Vn pBinary_$_[0] $f") && die $!;
                     system("cat $f.h >> $this->{dir}/$this->{name}BinaryShaders.cpp; rm -f $f.h");
                   },
            sub { system("rm -f $this->{dir}/$this->{name}BinaryShaders.cpp $this->{dir}/*.".$this->FileExt($stage));
                  $this->OpenFiles('f');
                  my $dxdir = `cygpath -u '$ENV{'DXSDK_DIR'}'`;
                  chomp($dxdir);
                  $ENV{'PATH'} .= ":$dxdir/Utilities/bin/x86";
                },
            sub { $this->WriteHeaders(); $this->CloseFiles('f'); }];
  }
  return $_[0]->Shaders::Output($_[1]);
}

new();
