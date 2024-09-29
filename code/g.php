<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
// Simple script to accept two uploaded files and two params
// 
  //phpinfo();
  $tests = array("a+b", "a-b", "a*b", "d+b", "d-b", "re"); // r = root a,b 1..10 D = 12 e = rand(1-9)^2
  class Captcha
  { 
  		public $picked;
		public $answer;
  		public $captchas;
	      function __construct(array $_captchas) {
			  $this->captchas=$_captchas;		  			  
    	}
		
		
		function convertNumberToWord($num = false)
		{
		$num = str_replace(array(',', ' '), '' , trim($num));
		if(! $num) {
			return false;
		}
		$num = (int) $num;
		$words = array();
		$list1 = array('', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten', 'eleven',
			'twelve', 'thirteen', 'fourteen', 'fifteen', 'sixteen', 'seventeen', 'eighteen', 'nineteen'
		);
		$list2 = array('', 'ten', 'twenty', 'thirty', 'forty', 'fifty', 'sixty', 'seventy', 'eighty', 'ninety', 'hundred');
		$list3 = array('', 'thousand', 'million', 'billion', 'trillion', 'quadrillion', 'quintillion', 'sextillion', 'septillion',
			'octillion', 'nonillion', 'decillion', 'undecillion', 'duodecillion', 'tredecillion', 'quattuordecillion',
			'quindecillion', 'sexdecillion', 'septendecillion', 'octodecillion', 'novemdecillion', 'vigintillion'
		);
		$num_length = strlen($num);
		$levels = (int) (($num_length + 2) / 3);
		$max_length = $levels * 3;
		$num = substr('00' . $num, -$max_length);
		$num_levels = str_split($num, 3);
		for ($i = 0; $i < count($num_levels); $i++) {
			$levels--;
			$hundreds = (int) ($num_levels[$i] / 100);
			$hundreds = ($hundreds ? ' ' . $list1[$hundreds] . ' hundred' . ' ' : '');
			$tens = (int) ($num_levels[$i] % 100);
			$singles = '';
			if ( $tens < 20 ) {
				$tens = ($tens ? ' ' . $list1[$tens] . ' ' : '' );
			} else {
				$tens = (int)($tens / 10);
				$tens = ' ' . $list2[$tens] . ' ';
				$singles = (int) ($num_levels[$i] % 10);
				$singles = ' ' . $list1[$singles] . ' ';
			}
			$words[] = $hundreds . $tens . $singles . ( ( $levels && ( int ) ( $num_levels[$i] ) ) ? ' ' . $list3[$levels] . ' ' : '' );
		} //end for loop
		$commas = count($words);
		if ($commas > 1) {
			$commas = $commas - 1;
		}
		return implode(' ', $words);
	}
		
		function pick() {
			return $this->captchas[rand(0,count($this->captchas)-1)];
		}
		
		function parse() {
			$this->answer = 0;
			$left =0;
			$right = 0;
			$operator=' ';
			$expr=$this->pick();
			$result = "";
			//echo "q=".$expr."<br>";
			foreach (str_split($expr) as $char) {
				switch ($char) {
					case 'a':
					  	$left = rand(1,15);
					    $result .= $this->convertNumberToWord($left);
					    break;						 
					case 'b':						
						$right = rand(1,10);
					    if ($operator=='-') { // No -ve numbers so make left bigger than right
							$left = $right+ rand(3,8);
							$result = $this->convertNumberToWord($left). " minus ".$this->convertNumberToWord($right);
						}
						else {
					      $result .= $this->convertNumberToWord($right);
						}
					    break;
					case 'e':
						$temp = rand(2,9);
					    $result .= $this->convertNumberToWord($temp*$temp);
					    break;						
					case '+':
						$result .= " plus ";
						$operator='+';
						break;
					case '-':
						$result .= " minus ";
						$operator='-';						
						break;		
					case 'd':
						$result .= " twelve ";
						$left= 12;
						break;
					case 'r':
						$result .= " square root of ";
						$left="N/A";
						$right="N/A";						
						$operator='r';						
						break;						
					case '*':
						$result .= " times ";
						$operator='*';						
						break;																	
				} // switch
			}	// for
			switch ($operator) {
				case '+':
				   $this->answer=$left+$right;
				   break;
				case '-':
				   $this->answer=$left-$right;
				   break;
				case '*':
				   if ($left > 10) {
					   $left=10;
					}
				   if ($right > 10) {
					   $right=10;
					}
				   $result = $this->convertNumberToWord($left). " times ".$this->convertNumberToWord($right);					   
				   $this->answer=$left*$right;
				   break;		
				case 'r':
				   $this->answer=$temp;
				   break;
			} // switch
			//echo "L=".$left." o=".$operator." r= ".$right;
			//echo " Answer=".$this->convertNumberToWord($answer)."<br>";
			return $result." equals?";		
		}
		
		function hashed($nonce) {
			return hash("ripemd256",$this->convertNumberToWord($nonce)."XYZ124");
		}

    function image($txt) {
	$img = imagecreate(500, 100);
	
	$textbgcolor = imagecolorallocate($img, 255, 255, 255);
	$textcolor = imagecolorallocate($img, 0, 0, 0);
	
	//imagestring($img, 5, 5, 5, $txt, $textcolor);
	$font = './Montserrat-Regular.ttf';
    imagettftext($img, 20, 0, 5, 90, $textcolor, $font, $txt);	
	imageline($img,0,80,500,80,$textcolor);	
	imageline($img,0,75,500,75,$textcolor);	
	imageline($img,0,85,500,85,$textcolor);	
	ob_start();
	imagepng($img);

	printf('<img src="data:image/png;base64,%s"/ width="300">', base64_encode(ob_get_clean()));
	imagedestroy($img);
	}
	
  } // class
?>