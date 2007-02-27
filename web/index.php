<?php
ini_set('display_errors','0');

////////////////////////////////////////////////////////
// Hack to redirect old forum urls (PNphpBB2) to the  //
// correct forum/topic in the SMF forum.              //
//                                                    //
// Inspired from  phpBB_Redirection_v1.0 hack         //
// by Spaceman-Spiff - spiff@monkey-pirate.com        //
////////////////////////////////////////////////////////
if(in_array($_SERVER['PHP_SELF'],array('/','/index.php')) &&
   in_array($_GET['file'],array('viewtopic','viewforum')))
{
    if ($_GET['name'] == 'PNphpBB2')
    {
        $forumurl = 'http://forums.codeblocks.org/';
        if ($_GET['file'] == 'viewtopic')
        {
            $x = (int) $_GET['t'];
            header ('Location: ' . $forumurl . 'index.php?topic=' . $x);
            exit;
        }
        else if ($_GET['file'] == 'viewforum')
        {
            $x = (int) $_GET['f'];
            header ('Location: ' . $forumurl . 'index.php?board=' . $x);
            exit;
        }
    }
}
// end of PNphpBB2 urls hack

require_once('prepend.php');
?>
<title>Code::Blocks IDE - Open Source, Cross-platform Free C++ IDE</title>
<h1>Code::Blocks</h1>

<div class="right">
<a href="/img/screenshots/scr1.png"><img width="312" height="229"
alt="Screenshot" title="Zoom in"
src="/img/screenshots/scr1_tiny.jpg" /><br />
<span class="caption">1.0RC2 (zoom in)</span></a><br />
<a class="caption" href="screenshots.php">(more screenshots)</a>
</div>
<h2 style="font-size:1.2em;border-bottom:none;">The open source, cross platform Free C++ IDE.</h2>

<p>Code::Blocks is a <em>free C++ IDE</em> built to meet the most demanding needs of its users.
It is designed to be very extensible and fully configurable.</p>

<p>Built around a plugin framework, Code::Blocks can be <em>extended with plugins</em>. It even
includes a plugin wizard so you can <em>create your own plugins</em>! (Free SDK downloaded separately)</p>

<p style="clear:right">Finally, an IDE with all the <a href="/features.php">features</a> <em>you</em> need,
having a consistent look, feel and operation across platforms.<br />
We hope you enjoy using Code::Blocks!</p>
<p>The Code::Blocks Team.</p>

<p><b><a href="/downloads.php">Download Code::Blocks now!</a></b></p>

<p class="caption"><b>[ <a title="Features list" href="/features.php">Next</a> ]</b></p>

<hr />


<p><b>Latest <a href="http://forums.codeblocks.org">forum</a> topics:</b></p>
<?php
include('http://forums.codeblocks.org/SSI.php?ssi_function=recentTopics');
?>
