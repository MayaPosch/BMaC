<?php
/*
	ota.php - Provides OTA firmware update functionality.
	
	Features:
		- Returns firmware image for specific UIDs.
		- Allows the managing of available firmware images.
		- Enables triggering of OTA updates by specific nodes
*/

// Configuration:
$firmware_folder = ''; 				// Must end with a slash if not empty.
$database_folder = '../databases/'; // Must end with a slash if not empty.

// Debug
ini_set('display_errors', '1');
ini_set('error_reporting', E_ALL);


// Use SQLite3 database. Creates if not present.
$is_new = false;
if (!file_exists($database_folder . 'firmware.db')) { $is_new = true; }
$db = new SQLite3($database_folder . 'firmware.db');

// Create new tables if new database.
if ($is_new) {
	$db->exec('CREATE TABLE IF NOT EXISTS firmware (uid TEXT UNIQUE, filename TEXT)');
}


// States:
// Default: 				list available firmware images.
// 'uid' query parameter:	return a firmware image for the UID, or a 404 error.
// 'update' (POST):			update the firmware image for the specified UID.
// 'trigger' (POST):		signal OTA update for specified UID.
// 'delete' (POST):			delete the specified UID from the database.
$has_error = false;
$errormsg = '';
if (isset($_GET['uid'])) {
	// Get filename for this UID.
	$uid = $_GET['uid'];
	$sql = 'SELECT * FROM firmware WHERE uid="' . $uid . '"';
	$result = $db->query($sql);
	if ($result === false) {
		var_dump(http_response_code(404));
		die();
	}
	
	$resultArr = $result->fetchArray();
	if ($resultArr === false) {
		var_dump(http_response_code(404));
		die();
	}
	
	$filepath = $firmware_folder . $resultArr['filename'];
	
	// Return the firmware image, if found. Otherwise a 404.
	if (!file_exists($filepath)) {
		var_dump(http_response_code(404));
		die();
	}
	
	header($_SERVER["SERVER_PROTOCOL"] . " 200 OK");
	header("Cache-Control: public"); // needed for internet explorer
	header("Content-Type: application/octet-stream");
	header("Content-Transfer-Encoding: Binary");
	header("Content-Length:" . filesize($filepath));
	header("Content-Disposition: attachment; filename={$resultArr['filename']}");
	readfile($filepath);
	die();
}
else if (isset($_GET['update'])) {
	$uid = $_GET['update'];
	$filename = $_GET['filename'];
	
	// If UID exists, overwrite with new filename, otherwise insert.
	$sql = 'INSERT OR REPLACE INTO firmware (uid, filename) VALUES ("' . $uid . '", "' . $filename . '")';
	$db->exec($sql);
}
else if (isset($_GET['trigger'])) {
	$uid = $_GET['trigger'];
	
	require('phpMQTT.php'); // Include the MQTT library.
	
	// Connect to the MQTT broker and publish the trigger for the UID.
	$mqtt = new phpMQTT("localhost", 1883, "phpMQTT");
	if (!$mqtt->connect(true, "phpMQTT RIP")) {
		$has_error = true;
		$errormsg = 'Failed to connect to the MQTT broker.';
	}
	
	$mqtt->publish("upgrade", $uid, 0);
	$mqtt->close();
}
else if (isset($_GET['delete'])) {
	$uid = $_GET['delete'];
	$sql = 'DELETE FROM firmware WHERE uid="' . $uid . '"';
	$db->exec($sql);
}

// Default state: show overview of the firmware images, along with management
// options.
?>
<!DOCTYPE html>
<html>
<head>
	<title>OtA Firmware Images</title>
</head>
<body>
<?php
if ($has_error) {
	echo '<p>Error: ' . $errormsg . '</p>';
}
?>

<table>
	<tr>
		<th>UID</th>
		<th>Firmware</th>
		<!--<th>Update firmware</th>-->
		<th>Trigger OTA</th>
		<th>Delete</th>
	</tr>
	<tr>
<?php

// Get current listing from the database.
// Columns are: 'uid' and 'filename'.
$result = $db->query('SELECT * FROM firmware');
if ($result == false) {
	echo 'Failed to read from database: ';
}

// Display the results.
$count = 0;
while ($row = $result->fetchArray()) {
	echo "<td>{$row['uid']}</td><td>{$row['filename']}</td>";
	echo '<td><a href="ota.php?trigger=' . $row['uid'] . '">Trigger</a></td>';
	echo '<td><a href="ota.php?delete=' . $row['uid'] . '">Delete</a></td></tr>';
	++$count;
}

// Scan firmware folder for images.
//$files = scandir($firmware_folder);
//if ($files === false) {
	// handle error
//}

// 
?>
	</tr>
</table>
<br>
Results: <?php echo $count; ?><br>
<br>
Update/Add firmware for a (new) UID:<br>
1. Upload firmware image to firmware folder: <?php echo $firmware_folder; ?><br>
2. Input details below, and submit.
<form action="ota.php" method="get">
	UID<br>
	<input type="text" name="update"><br>
	Filename<br>
	<input type="text" name="filename"><br>
	<br>
	<input type="submit" value="Submit">
</form>

</body>
</html>
