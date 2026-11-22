"""pil_nearest: Pillow's nearest-neighbour resize, kept working where a
Pillow build breaks it.

Ubuntu's python3-pil 12.1.1-2ubuntu1.5 (a security update of 15 November
2026) fails every `Image.resize(size, Image.NEAREST)` with "SystemError:
<method 'resize' of 'ImagingCore' objects> returned NULL", in every mode,
while bilinear resizes, crops and transforms still work. The project's
pictures are scaled that way everywhere (play.py's screen, the cyberworld
MCP's pictures, the screenshots, the trailers, the art), so one broken
build stopped every playtest.

Imported once, this module wraps `Image.Image.resize`: a nearest resize
that fails so is done again as an affine transform with nearest sampling,
which takes the same source pixel for every output pixel (whole-number
scales exactly, the only ones the project uses). Any other resize, and
any other error, goes through unchanged.

    import pil_nearest  # noqa: F401  (tools/ on sys.path)

Standard library and Pillow only.
"""
from PIL import Image

_NEAREST = (Image.NEAREST, getattr(getattr(Image, 'Resampling', Image), 'NEAREST', Image.NEAREST))
_resize = Image.Image.resize


def nearest_by_transform(im, size):
    """`im` scaled to `size` with nearest sampling, by an affine transform."""
    w, h = int(size[0]), int(size[1])
    return im.transform((w, h), Image.AFFINE, (im.width / w, 0, 0, 0, im.height / h, 0), Image.NEAREST)


def _is_nearest(im, resample):
    # (Pillow's default for "1" and "P" images is nearest)
    return resample in _NEAREST or (resample is None and im.mode in ('1', 'P'))


def _resize_kept(self, size, resample=None, box=None, reducing_gap=None):
    try:
        return _resize(self, size, resample, box, reducing_gap)
    except SystemError:
        if box is not None or not _is_nearest(self, resample):
            raise
        return nearest_by_transform(self, size)


if getattr(Image.Image.resize, '__name__', '') != '_resize_kept':
    Image.Image.resize = _resize_kept
