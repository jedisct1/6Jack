#! /usr/bin/env php
<?
    $unpacker = new MessagePackUnpacker();
    $buffer = "";
    $nread = 0;
    $false = 0;
    
    while (1) {
        $data = fread(STDIN, 65536);
        if ($data === FALSE || feof(STDIN)) break;
        
        $buffer = $buffer . $data;
        while(true) {
            if($unpacker->execute($buffer, $nread)){
                $msg = $unpacker->data();

                fwrite(STDERR, print_r($msg, true));
                
                fwrite(STDOUT, msgpack_pack($msg));
                $unpacker->reset();
                $buffer = substr($buffer, $nread);
                $nread = 0;

                if (!empty($buffer)){
                    continue;
                }
            }
            break;
        }
    }
  
?>
