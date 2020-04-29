<?php

$food = $_GET['query'];

$food = str_replace("_","%20",$food);

#echo $food;

$ch = curl_init("https://xiuimsth33.execute-api.us-east-2.amazonaws.com/test/helloworld?query=%22{$food}%22");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_BINARYTRANSFER, true);
$content = curl_exec($ch);
curl_close($ch);
echo $content;




?>

