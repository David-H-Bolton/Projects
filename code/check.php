<?php

if ($_SERVER["REQUEST_METHOD"] == "POST") {
  // collect value of input field
  //print_r($_POST);
  include "g.php";  
  $c= new Captcha(array(""));  
  $answer = strtolower(trim($_POST['answer']));
  if (empty($answer)) {
    echo "No answer provided";
  } else {
	  $correct=trim($_POST['correct']);
	if ($c->hashed($answer)== $correct){
		  echo "You got it right!";
	  }
	  else {
		  //echo "L(answer)=".strlen($answer)." L(correct)=".strlen($correct);
		  echo "You got it wrong. You answered ".$answer."<br>Correct answer was ".$correct;
	  }
  }
}
?>
