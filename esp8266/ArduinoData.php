<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" dir="ltr" lang="en" xmlns:fb="http://ogp.me/ns/fb#" >
<head>
<meta http-equiv="refresh" content="60">
<title>Arduino Test Bed</title>


<script src="http://code.jquery.com/jquery-1.11.0.min.js"></script>
<script type="text/javascript">
$("document").ready(function(){
  $(".js-ajax-options").submit(function(){
    var data = {
      "action": "test"
    };
    data = $(this).serialize(); // + "&" + $.param(data);
    $.ajax({
      type: "POST",
      //dataType: "json",
      url: "data_store1.php", //Relative or absolute path to data_parser.php file
      data: data
    })
     .done(function(data) {
        $(".the-return").html(data);
        //alert("Form submitted successfully.\nReturned json: " + data["json"]);
      })
      .fail(function() {
      		alert( "Posting failed.");
      });
    
    return false;
  });
});
</script>

<?php
	$file = 'temp.txt';
	date_default_timezone_set('Australia/Melbourne');
	if (file_exists($file)){
		$tempdata = file_get_contents($file);
		$lastmodified = "Last captured: ".date ("d M y h:i:s. A", filemtime($file));
	}
?>
<script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
<script type="text/javascript">
      google.charts.load('current', {'packages':['gauge']});
      google.charts.setOnLoadCallback(drawChart);
     function drawChart() {

        var data = google.visualization.arrayToDataTable([
          ['Label', 'Value'], 
          ['Temperature', 
<?php    
    echo $tempdata;
?>        
          ],]);
  // setup the google chart options here
  // width: 400, height: 120,
      var options = {
       
        redFrom: 35, redTo: 100,
        yellowFrom: 0, yellowTo: 15,
        greenFrom: 16, greenTo: 34,
         minorTicks: 5    };
     var chart = new google.visualization.Gauge(document.getElementById('chart_div'));

    chart.draw(data, options);

 	function resizeHandler () {
        chart.draw(data, options);
    }
    if (window.addEventListener) {
        window.addEventListener('resize', resizeHandler, false);
    }
    else if (window.attachEvent) {
        window.attachEvent('onresize', resizeHandler);
    }
    
    setInterval(function() {
    data.setValue(0, 1, 
<?php    
    echo $tempdata;
?> 
	);
    chart.draw(data, options);
      }, 13000);

    }
</script>

</head>

<body>

Arduino Data <br>


<!--Put the following in the <body>-->
<!--<form action="ArduinoData.php" class="js-ajax-options" method="post" accept-charset="utf-8">
  <input type="text" name="temp_sensor" value="15" placeholder="Temp Sensor" />
  <input type="submit" name="submit" value="Update"  />
</form>


<div class="the-return">
  [HTML is replaced when successful.]
</div> -->
<!--// below is the google chart html-->
<!--<div id="chart_div" style="width: 300px; height: 120px;"></div>-->
<div id="chart_div" style="width: 100%; height: 100%;"></div>
<?php
	echo $lastmodified;
?> <br>

</body></html>
