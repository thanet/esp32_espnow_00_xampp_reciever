<?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $id = $_POST['id'];
    $temperature = $_POST['temperature'];
    $humidity = $_POST['humidity'];
    $readingId = $_POST['readingId'];

    // Print or store the received data
    echo "Data received: ID = $id, Temperature = $temperature, Humidity = $humidity, Reading ID = $readingId";

    // Implement database storage or further processing here
} else {
    echo "Invalid request";
}
?>
