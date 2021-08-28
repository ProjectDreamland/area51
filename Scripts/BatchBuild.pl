use strict;
use Net::SMTP;

# ------------------------------------------------------------------------
# Batch code build script
# ------------------------------------------------------------------------

my @buildreport;
my $email_to        = $ENV{"USERNAME"};
my $email_from      = $email_to;
my $email_smtp_host = 'hermes.inevitable.com';

# ------------------------------------------------------------------------
# Email
# ------------------------------------------------------------------------

sub Email
{
    my( $subject, @body ) = @_;

    my $smtp = Net::SMTP->new($email_smtp_host, Timeout => 30) || die "Can't connect to $email_smtp_host.\n";

    $smtp->mail     ( $email_from ) || die "cannot mail from.\n\n";
    $smtp->recipient( ($email_to) );
    $smtp->data     ( );
    $smtp->datasend ( "To:\t$email_to\n" );
    $smtp->datasend ( "Subject:\t$subject\n\n" );

    foreach my $S ( @body )
    {
        $smtp->datasend ( "$S" );
    }

    $smtp->dataend;
    $smtp->quit;
}
# -----------------------------------------------------------------------------
# Execute and return error code and captured STDOUT / STDERR as a list
# -----------------------------------------------------------------------------

sub Execute
{
    my( $command ) = @_;
    my @output;
    
    # Execute and capture stdout
    @output = `$command 2>&1`;

    # TODO: Strip any trailing newlines from the output

    return( $? >> 8, @output );
}

# -----------------------------------------------------------------------------
# Build a visual studio.net 2003 sln and configuration
# -----------------------------------------------------------------------------

sub Build
{
    my( $command ) = @_;
    my @output;
    my $err;
    
    push( @buildreport, "#==============================================================================\n" );
    push( @buildreport, "# " . $command . "\n" );
    push( @buildreport, "#==============================================================================\n" );
    
    ( $err, @output ) = Execute( '"C:/Program Files/Microsoft Visual Studio .NET 2003/Common7/IDE/devenv.com" ' . $command );
    push( @buildreport, @output );
}

# -----------------------------------------------------------------------------
# Do the builds
# -----------------------------------------------------------------------------

Build( 'C:/Projects/A51/Apps/GameApp/A51.sln /build "PS2-Debug"' );
Build( 'C:/Projects/A51/Apps/GameApp/A51.sln /build "PS2-OptDebug"' );
Build( 'C:/Projects/A51/Apps/GameApp/A51.sln /build "Win32-Debug"' );
Build( 'C:/Projects/A51/Apps/GameApp/A51.sln /build "Win32-OptDebug"' );

Email( "Batch Build Report", @buildreport );
