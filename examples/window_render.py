# -*- encoding: utf-8 -*-

"""Render an image to a window

This is adapted from the SDL 2 example at
<http://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world>.
"""

import pygame
from pygame.locals import (RENDERER_ACCELERATED, RENDERER_PRESENTVSYNC)
import os


def get_resource_path():
    my_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(my_dir, 'data')


def run():
    pygame.display.init()
    title = "Hello World!"
    rect = (100, 100, 640, 480)
    with pygame.display.Window(title, rect) as win:
        flags = RENDERER_ACCELERATED | RENDERER_PRESENTVSYNC
        ren = win.create_renderer(flags=flags)
        image_path = os.path.join(get_resource_path(), 'hello.bmp')
        bmp = pygame.image.load(image_path)
        tex = ren.create_texture_from_surface(bmp)

        for i in range(3):
            ren.clear()
            tex.copy_to_renderer()
            ren.update()
            pygame.time.delay(1000)
    pygame.quit()

if __name__ == '__main__':
    run()
