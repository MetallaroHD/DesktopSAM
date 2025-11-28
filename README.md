DesktopSAM is a C++ app implementing efficientvit-segmentanything.

How to use:
1) Download an encoder and the corresponding decoder from https://huggingface.co/mit-han-lab/efficientvit-sam/tree/main/onnx,
   refer to https://huggingface.co/mit-han-lab/efficientvit-sam/blob/main/README.md for choosing the best model for your needs.
   NOTE: for now it only supports XL0 and XL1 models. Support for different models will be added later.
3) Launch the application.
4) Set paths to the encoder/decoder and select [Start].
5) Load an image of your choice.
6) The [Segment] button allows to pick a point on the image and start segmentation.
7) Using the left/right mouse buttons you may manually select areas you want to save/discard.
8) Once satisfied press [Save] to save the resulting image.

Dependencies:
1) Qt
2) OpenCV
3) onnxruntime (with DirectML)
