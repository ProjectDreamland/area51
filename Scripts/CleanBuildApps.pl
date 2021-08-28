use strict;
use Win32::NetAdmin;
use Net::SMTP;

# ------------------------------------------------------------------------
# main
# ------------------------------------------------------------------------

# Read user name from environment
my $username = $ENV{"USERNAME"};
if( !defined( $username ) )
{
    die "Could not determine USERNAME from environment\n";
}

# Read build script
open( FILE, "< CleanBuildScript.txt" ) || die "Can't open CleanBuildScript\n";
my @script = <FILE>;
close FILE;
my $script = join( "", @script );

# Setup the email
my $from = "$username\@inevitable.com";
my @recip_list = ('build@a51apps-01');
my $smtp_host = "a51apps-01";
my $smtp = Net::SMTP->new($smtp_host, Timeout => 30) || die "Can't connect to $smtp_host.\n";
$smtp->mail     ( $from ) || die "cannot mail from.\n\n";
$smtp->recipient( @recip_list );
$smtp->data     ( );
$smtp->datasend ( "To:\tBuild\n" );
$smtp->datasend ( "Subject:\tBuild\n" );
$smtp->datasend ( $script );
$smtp->dataend;
$smtp->quit;
