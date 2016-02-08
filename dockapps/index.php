<?php
$dockapps = parse_ini_file("dockapps.db", true);

$name = $_GET["name"];
$category = $_GET["category"];
$download = $_GET["download"];

function getCategory($dockapp) {
	return $dockapp["category"];
}

if ($download) {
	$download=str_replace(" ", "+", $download);
	preg_match("/([\w\-+.]+)-([\w+.]+).tar.gz/", $download, $matches);
	header("Location: http://repo.or.cz/w/dockapps.git/snapshot/" .
	       $dockapps[$matches[1]]["version-$matches[2]"] . ".tar.gz");
	die();
}
?>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: Dockapps</title>
  <meta http-equiv="Content-Type" content="text/html; charset=us-ascii" />
  <link href="../title.css" rel="stylesheet" type="text/css" />
</head>
<?php
echo "<body>\n";
include("../dock.php");
include("../header.php");?>
 <div>
    <table class="inner" border="0" cellpadding="1" cellspacing="1">
      <tr>
        <td content="content" colspan="2" valign="top">
          <br />
          <br />
<h1><a href=".">Dockapps</a></h1>
<?php
if ($name) {
	$hosted = $dockapps[$name]["hosted"];
	echo "<h2>$name</h2>\n";
	echo "<b>Category:</b> <a href=\"?category=" .
		$dockapps[$name]["category"] . "\">" .
		$dockapps[$name]["category"] .
		"</a><br/>\n";
	if ($dockapps[$name]["url"]) {
		echo "<b>";
		if ($hosted)
			echo "Original ";
		echo "Website</b>: <a href=\"" . $dockapps[$name]["url"] .
			"\">" . $dockapps[$name]["url"] . "</a><br/>\n";
	}
	echo "<b>Description:</b> " . nl2br($dockapps[$name]["description"]) .
		"<br/>\n";
	$images = explode(",", $dockapps[$name]["image"]);
	if ($images) {
	   	echo "<h3>Screenshots:</h3>\n";
		foreach ($images as $image)
			echo "<img src=\"img/$image\"/>\n";
	}
	if ($hosted) {
		echo "<h3>Available Versions:</h3>\n";
		$versions = array();
		$info = $dockapps[$name];
		foreach ($info as $key => $value) {
			if (substr($key, 0, 8) == "version-")
				array_push($versions,
				substr($key, strrpos($key, "-")+1));
		}
		foreach ($versions as $version) {
			echo "<a href=\"?download=$name-$version.tar.gz\">" .
			"$name-$version</a><br/>\n";
		}
		echo "<p/>\n<i>Note:</i>  By default, the above tarballs will ";
		echo 'save as dockapps-$string.tar.gz, where $string is a ';
		echo "long string of hexadecimal characters.  You may wish to ";
		echo "right click and use the <i>Save Link As...</i> option.\n";
	} else {
		echo "<p/>\nThis dockapp is not maintained by the Window ";
		echo "Maker Developers Team.	Please visit the dockapp's ";
		echo "website for downloads.\n";
	}
?>
<p/>

<?php
} else if ($category) {
	echo "<h2>$category</h2>\n";
	$matches = array();
	foreach ($dockapps as $dockapp => $info)
		if ($info["category"] == $category)
			array_push($matches, $dockapp);
	sort($matches);
	foreach ($matches as $dockapp)
		echo "<a href=\"?name=" . urlencode($dockapp) .
		"\">$dockapp</a><br/>\n";

} else {?>
Here you can download git snapshots of a number of Window Maker dockapps which
are no longer maintained by their original developers and have been adopted by
the Window Maker Developers Team.  You can browse the repository itself at
<a href="http://repo.or.cz/dockapps.git">http://repo.or.cz/dockapps.git</a>.
<p/>
If you would like to submit a patch to an existing dockapp or add a new dockapp
to the repository, please follow <a href="http://repo.or.cz/w/wmaker-crm.git/blob/HEAD:/The-perfect-Window-Maker-patch.txt">these directions</a>.
<p/>
Some of the dockapps listed are not maintained by the Window Maker Developers
Team.  You may download these dockapps at their respective websites.
<h2>Categories</h2>
<?php
	$categories = array_unique(array_map("getCategory", $dockapps));
	sort($categories);
	foreach ($categories as $category)
		echo "<a href=\"?category=$category\">$category</a><br/>\n";
?>
<p/>
Many other dockapps may be found at <a href="http://www.cs.mun.ca/~gstarkes/wmaker/dockapps/">http://www.cs.mun.ca/~gstarkes/wmaker/dockapps/</a>.
<?php
}?>
        </td>
      </tr>
    </table>

<?php
include("../footer.php");
?>
