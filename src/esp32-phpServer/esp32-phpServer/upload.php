<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $inputData = file_get_contents('php://input');

    // Dosya ismi ve yolu
    $fileName = 'kayit.wav';
    $filePath = __DIR__ . '/' . $fileName;

    // WAV başlığı oluşturma
    $sampleRate = 16000;
    $bitsPerSample = 16;
    $numChannels = 1;
    $byteRate = $sampleRate * $numChannels * $bitsPerSample / 8;
    $blockAlign = $numChannels * $bitsPerSample / 8;

    $header = pack('N', 0x52494646); // "RIFF"
    $header .= pack('V', 36 + strlen($inputData)); // File size - 8
    $header .= pack('N', 0x57415645); // "WAVE"
    $header .= pack('N', 0x666d7420); // "fmt "
    $header .= pack('V', 16); // Subchunk1Size (16 for PCM)
    $header .= pack('v', 1); // AudioFormat (1 for PCM)
    $header .= pack('v', $numChannels); // NumChannels
    $header .= pack('V', $sampleRate); // SampleRate
    $header .= pack('V', $byteRate); // ByteRate
    $header .= pack('v', $blockAlign); // BlockAlign
    $header .= pack('v', $bitsPerSample); // BitsPerSample
    $header .= pack('N', 0x64617461); // "data"
    $header .= pack('V', strlen($inputData)); // Subchunk2Size

    // WAV dosyasını yazma
    file_put_contents($filePath, $header . $inputData);

    echo "File saved successfully: $fileName";

    include("api.php");
} else {
    echo "Invalid request method";
}
?>