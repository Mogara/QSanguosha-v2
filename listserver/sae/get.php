<?php
require('settings.php');
$storage=new SaeStorage($access_key,$secret_key);
echo $storage->getUrl($domain,'servers');
?>