<?php
function version_parse($file)
{
    $info = array();
    $fh = fopen($file,'r');
    while(!feof($fh))
    {
        $str = fgets($fh);
        $pos = strpos($str,'=');
        if($pos != FALSE)
        {
            $head = substr($str,0,$pos);
            if($head == 'bugfixes')
            {
                $bugfixes = substr($str,$pos+1);
                while(!feof($fh))
                    $bugfixes = $bugfixes . "<br />" . fgets($fh);
                $info['bugfixes']=$bugfixes;
       	    }
            else
                $info[$head]=substr($str,$pos+1);
        }
    }
    fclose($fh);
    return $info;
}
?>

