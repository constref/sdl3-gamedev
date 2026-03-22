flatc -n -o ..\..\..\EngineEditor\AvaloniaEditor\FlatBuffers\ .\engine.fbs .\usd.fbs .\editor_envelope.fbs .\engine_envelope.fbs .\piped_envelope.fbs
flatc -c -o . .\engine.fbs .\usd.fbs .\editor_envelope.fbs .\engine_envelope.fbs .\piped_envelope.fbs
