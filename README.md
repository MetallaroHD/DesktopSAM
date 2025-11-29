DesktopSAM is a C++ app implementing efficientvit-segmentanything.

NOTE: DesktopSAM currently supports models XL0 and XL1. Support for other models will be implemented soon.

How to use:
1) Download an encoder and the corresponding decoder from https://huggingface.co/mit-han-lab/efficientvit-sam/tree/main/onnx,
   refer to https://huggingface.co/mit-han-lab/efficientvit-sam/blob/main/README.md for choosing the best model for your needs.
2) Launch the application.
3) Set paths to the encoder/decoder and select [Start].
4) Load an image of your choice.
5) The [Segment] button allows to pick a point on the image and start segmentation.
6) Using the left/right mouse buttons you may manually select areas you want to save/discard.
7) You can hold and drag with the mouse wheel to pan the view.
8) Once satisfied press [Save] to save the resulting image.

Dependencies:
1) Qt
2) OpenCV
3) onnxruntime (with DirectML)
