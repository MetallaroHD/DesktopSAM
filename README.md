BackgroundRemover is a C++ app implementing efficientvit-segmentanything.

How to use:
1) Download an encoder and the corresponding decoder from https://huggingface.co/mit-han-lab/efficientvit-sam/tree/main/onnx,
   refer to https://huggingface.co/mit-han-lab/efficientvit-sam/blob/main/README.md for choosing the best model for your needs.
2) Launch the application.
3) Optional: launch [Test GPU] to check if your system is suitable for GPU computation. This may speed up the segmentation significantly.
4) Set paths to the encoder/decoder and the image you wish to segment and select [Start].
5) Select a point on the object you wish to segment and check the result preview. Once satisfied, press save to export the result image.

Dependencies:
1) Qt
2) OpenCV
3) onnxruntime (with DirectML)
