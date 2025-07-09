#pragma once

#include "ui.h"
#include <math.h>

template<typename Buffer>
struct SpriteT {
    const Buffer &src;
    const fb x0, y0, x1, y1;
    SpriteT(const Buffer &src, const fb &x0, const fb &y0, const fb &x1, const fb &y1)
        :src(src),x0(x0),y0(y0),x1(x1),y1(y1) {}

    inline constexpr fb getWidth() const { return x1 - x0; }
    inline constexpr fb getHeight() const { return y1 - y0; }
    inline constexpr auto getSize() const { return {this->getWidth(), this->getHeight()}; }
};

template<typename Buffer>
struct TextureT : public Buffer {
    using Sprite = SpriteT<Buffer>;

    inline void clear(const pixel &px) {
        pixel d = 0;
        for (fb i = 0; i < this->PXPERBYTE; i++)
            d |= px << (this->bpp * i);
        for (fb i = 0; i < this->getSize(); i++) {
            this->buffer[i] = d;
        }
    }

    inline void fill(const fb &x0, const fb &y0, const fb &x1, const fb &y1, const pixel &px) {
        for (fb x = x0; x < x1; x++)
            for (fb y = y0; y < y1; y++)
                this->putPixel(x, y, px);
    }

    inline void circle(const fb &cx, const fb &cy, const fb &r, const pixel &px, const bool fill=true) {
        const fb r2 = r * r;

        for (fb x = cx - r; x < cx + r + 1; x++) {
            for (fb y = cy - r; y < cy + r + 1; y++) {
                const fb xy2 = (x-cx) * (x-cx) + (y-cy) * (y-cy);
                //if (!fill && ((xy2 - r2) > 2 || (xy2 - r2) < -2))
                if (!fill && abs(xy2 - r2) > 2)
                    continue;
                if (xy2 < r2)
                    this->putPixel(x, y, px);
            }
        }
    }

    inline void putSprite(const Sprite &sprite, const fb &x, const fb &y, const fb &w = 0, const fb &h = 0) {
        const fb width = w ? w : sprite.getWidth();
        const fb height = h ? h : sprite.getHeight();

        for (fb i = 0, k = x; i < w; i++, k++) {
            for (fb j = 0, l = y; j < h; j++, l++) {
                this->putPixel(k, l, sprite.src.getPixel(i + sprite.x0, j + sprite.y0));
            }
        }
    }

    template<typename calc=short, typename CALLBACK>
    inline void line_callback(const fb &x1, const fb &y1, const fb &x2, const fb &y2, CALLBACK callback) {
        calc x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;
		
		dx=calc(x2)-x1;
		dy=calc(y2)-y1;
		
		dx1=abs(dx);
		dy1=abs(dy);
		
		px=2*dy1-dx1;
		py=2*dx1-dy1;

		if(dy1<=dx1) {
			if(dx>=0) {
				x=x1;
				y=y1;
				xe=x2;
			} else {
				x=x2;
				y=y2;
				xe=x1;
			}
            
            callback(x,y);
			
			for(i=0;x<xe;i++) {
				x=x+1;
				if(px<0) {
					px=px+2*dy1;
				} else {
					if((dx<0 && dy<0) || (dx>0 && dy>0)) {
						y=y+1;
					} else {
						y=y-1;
					}
					px=px+2*(dy1-dx1);
				}

				callback(x,y);
			}
		} else {
			if(dy>=0) {				
				x=x1;
				y=y1;
				ye=y2;
			} else {
				x=x2;
				y=y2;
				ye=y1;
			}  
			
            callback(x,y);
  
			for(i=0;y<ye;i++) {
				y=y+1;
				if(py<=0) {
					py=py+2*dx1;
				} else {
					if((dx<0 && dy<0) || (dx>0 && dy>0)) {
						x=x+1;
					} else {
						x=x-1;
					}
					
					py=py+2*(dx1-dy1);
				}	
				
                callback(x,y);
			}
		}
    }

    inline void line(const fb &x1, const fb &y1, const fb &x2, const fb &y2, const pixel &px) {
        line_callback(x1, y1, x2, y2, 
            [this, px](const fb &x, const fb &y) { this->putPixelBound(x, y, px); }
        );
    }
};

template<typename Buffer>
struct AtlasT : public TextureT<Buffer> {
    using Sprite = SpriteT<Buffer>;

    inline constexpr Sprite getSprite(const fb &x, const fb &y, const fb &sx, const fb &sy) const {
        return Sprite(*this, x, y, x+sx, y+sy);
    }
};