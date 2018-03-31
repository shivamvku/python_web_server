<?php

$theparam = $_POST;
$file = './temp.txt';
// The new person to add to the file
$temperature = $theparam["temp_sensor"];
// Write the contents to the file, 
// using the FILE_APPEND flag to append the content to the end of the file
// and the LOCK_EX flag to prevent anyone else writing to the file at the same time
echo file_put_contents($file, $temperature, LOCK_EX);

?>