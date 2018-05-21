#!/usr/bin/php

<?php
$server_ip   = '127.0.0.1';
$server_port = 10000;
$beat_period = 5;
$message     = '51';
	
print "Sending heartbeat to IP $server_ip, port $server_port\n";
print "press Ctrl-C to stop\n";

if ($socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP)) {
  while (1) {
    socket_sendto($socket, $message, strlen($message), 0, $server_ip, $server_port); 
    print "Time: " . date("%r") . "\n";
    sleep($beat_period);
  }
} else {
  print("can't create socket\n");
}
?>



