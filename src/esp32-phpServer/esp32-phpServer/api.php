<?php
// Azure API bilgileri
$speechRegion = "eastus";
$speechKey = "your-azure-speech-key";
$language = "tr-TR";

// Çalışma dizinindeki ilk .wav dosyasını bul
$wavFile = __DIR__ . "/kayit.wav";

if (empty($wavFile)) {
    die("Çalışma dizininde .wav dosyası bulunamadı.\n");
}

// Azure Speech API isteği için URL
$url = "https://$speechRegion.stt.speech.microsoft.com/speech/recognition/conversation/cognitiveservices/v1?language=$language&format=detailed";

// API isteği için başlıklar
$headers = [
    "Ocp-Apim-Subscription-Key: $speechKey",
    "Content-Type: audio/wav"
];

// .wav dosyasını okuyarak API'ye gönder
$audioData = file_get_contents($wavFile);

$options = [
    "http" => [
        "header"  => implode("\r\n", $headers),
        "method"  => "POST",
        "content" => $audioData
    ]
];

$context = stream_context_create($options);
$response = file_get_contents($url, false, $context);

if ($response === FALSE) {
    die("Azure API çağrısı başarısız.\n");
}

$responseData = json_decode($response, true);

// "DisplayText" objesini al
if (isset($responseData['DisplayText'])) {
    $result = $responseData['DisplayText'];
} else {
    die("Azure API yanıtında DisplayText bulunamadı.\n");
}

// Gemini AI API bilgileri
$geminiApiKey = "your-gemini-api-key"; // Burada kendi API anahtarınızı kullanın
$geminiApiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=$geminiApiKey";

// Gemini API'ye gönderilecek veri
$geminiPayload = [
    "contents" => [
        [
            "parts" => [
                ["text" => $result."\n\nvereceğin cevap maksimum 200 karakter uzunluğunda olsun"]
            ]
        ]
    ],
    "generationConfig" => [
        "maxOutputTokens" => 800,
        "topP" => 0.8,
        "topK" => 10
    ]
];


// Gemini API isteği
$geminiHeaders = [
    "Content-Type: application/json"
];

$options = [
    "http" => [
        "header"  => implode("\r\n", $geminiHeaders),
        "method"  => "POST",
        "content" => json_encode($geminiPayload)
    ]
];

$context = stream_context_create($options);
$geminiResponse = file_get_contents($geminiApiUrl, false, $context);

if ($geminiResponse === FALSE) {
    die("Gemini API çağrısı başarısız.\n");
}

$geminiResponse = json_decode($geminiResponse, true);

// "text" objesini al
if (isset($geminiResponse['candidates'][0]['content']['parts'][0]['text'])) {
    $text = $geminiResponse['candidates'][0]['content']['parts'][0]['text'];
} else {
    echo "Text objesi bulunamadı.\n";
}

// Azure TTS API isteği
$ttsUrl = "https://$speechRegion.tts.speech.microsoft.com/cognitiveservices/v1";
$ttsHeaders = [
    "Ocp-Apim-Subscription-Key: $speechKey",
    "Content-Type: application/ssml+xml",
    "X-Microsoft-OutputFormat: riff-16khz-16bit-mono-pcm",
    "User-Agent: curl"
];

$ssml = "<speak version='1.0' xml:lang='$language'><voice xml:lang='$language' xml:gender='Male' name='tr-TR-AhmetNeural'>$text</voice></speak>";

$options = [
    "http" => [
        "header"  => implode("\r\n", $ttsHeaders),
        "method"  => "POST",
        "content" => $ssml
    ]
];

$context = stream_context_create($options);
$ttsResponse = file_get_contents($ttsUrl, false, $context);

if ($ttsResponse === FALSE) {
    die("Azure TTS API çağrısı başarısız.\n");
}

// Yanıtı "yanıt.wav" dosyasına yaz
$filePath = __DIR__ . "/yanit.wav";
if (file_put_contents($filePath, $ttsResponse) === FALSE) {
    die("Yanıt dosyaya yazılamadı.\n");
}

echo "Ses dosyası kaydedildi: $filePath\n";
?>