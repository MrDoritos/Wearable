#pragma once

#include "framebuffer.h"
#include "sizes.h"
#include <math.h>

namespace wbl {

template<typename BufferT>
struct SpriteT : public Size {
    using Buffer = BufferT;
    const Buffer *src;

    constexpr SpriteT(const Buffer *src, const Size &size)
        :Size(size),src(src) { }
    constexpr SpriteT(const Buffer &src, const Size &size)
        :Size(size),src(&src) { }
    constexpr SpriteT(const Buffer *src, const fb &x, const fb &y, const fb &w, const fb &h)
        :Size(x,y,w,h),src(src) { }
};

struct IGraphicsContext;

using Sprite = SpriteT<IGraphicsContext>;

struct IGraphicsContext {
    virtual inline void putPixel(const Origin &pos, const pixel &px) = 0;
    virtual inline void putPixel(const fb &x, const fb &y, const pixel &px) = 0;
    virtual inline void fill(const Size &size, const pixel &px) = 0;
    virtual inline pixel getPixel(const Origin &pos) const = 0;
    virtual inline pixel getPixel(const fb &x, const fb &y) const = 0;
    virtual inline void circle(const Origin &center, const fb &radius, const pixel &px, const bool &fill=true) = 0;
    virtual inline bool isBound(const Origin &pos) const = 0;
    virtual inline Length getLength() const = 0;
    virtual inline void putTexture(const IGraphicsContext *texture, const Size &texture_size, const Origin &destination_origin) = 0;
    virtual inline void putSprite(const Sprite &sprite, const Origin &pos) = 0;
    virtual inline void line(const Origin &pos_start, const Origin &pos_end, const pixel &px) = 0;
    virtual inline fb getAlphaTest() const = 0;
};

template<typename Buffer>
struct TextureT : public Buffer {
    using Buffer::Buffer;

    using Sprite = SpriteT<Buffer>;

    constexpr inline void clear(const pixel &px = 0) {
        const fb len = this->getSize();
        pixel d = 0;
        for (fb i = 0; i < this->PXPERBYTE; i++)
            d |= px << (this->BPP * i);
        for (fb i = 0; i < len; i++) {
            this->buffer[i] = d;
        }
    }

    constexpr inline void fill(const fb &x0, const fb &y0, const fb &x1, const fb &y1, const pixel &px) {
        for (fb x = x0; x < x1; ++x)
            for (fb y = y0; y < y1; ++y)
                this->putPixel(x, y, px);
    }

    constexpr inline void border(const Size &size, const pixel &px) {
        for (fb x = size.x; x < size.x + size.width; x++) {
            this->putPixel(x, size.y, px);
            this->putPixel(x, size.y+size.height-1, px);
        }

        for (fb y = size.y + 1; y < size.y + size.height;y++) {
            this->putPixel(size.x, y, px);
            this->putPixel(size.x+size.width-1,y,px);
        }
    }

    constexpr inline void difference(const Size &outer, const Size &inner, const pixel &px) {
        for (fb x = outer.x; x < outer.x + outer.width; x++)
            for (fb y = outer.y; y < outer.y + outer.height; y++)
                if (!inner.isBound(x, y))
                    this->putPixel(x, y, px);
    }

    constexpr inline void fill(const Size &size, const pixel &px) {
        fill(size.x, size.y, size.x + size.width, size.y + size.height, px);
    }

    template<typename IType=fb, typename CALLBACK>
    constexpr inline void fill_callback(CALLBACK callback, const IType &x0, const IType &y0, const IType &x1, const IType &y1) {
        for (IType x = x0; x < x1; x++)
            for (IType y = y0; y < y1; y++)
                callback(x, y);
    }

    template<typename calc=short, typename FType=fb, typename ORIGIN_T=Origin, typename IType= typename ORIGIN_T::value_type>
    constexpr inline void circle(const ORIGIN_T &center, const FType &radius, const pixel &px, const bool fill=true) {
        circle<calc,IType,FType>(center.x, center.y, radius, px, fill);
    }

    template<typename calc=short, typename IType=fb, typename FType=fb, typename CALLBACK>
    constexpr inline void circle_callback(CALLBACK callback, const IType &cx, const IType &cy, const FType &radius, const bool &fill=true) {
        const calc r = calc(radius);
        const calc r2 = r * r;

        for (IType x = cx - r; x < cx + r + 1; x++) {
            for (IType y = cy - r; y < cy + r + 1; y++) {
                const calc xd = calc(x) - cx;
                const calc yd = calc(y) - cy;
                const calc xy2 = (xd * xd) + (yd * yd);
                const calc xyd = xy2 - r2;
                if (!fill && (xyd >= 0 ? xyd : -xyd) > (r*2))
                    continue;
                if (xy2 < r2)
                    callback(x, y);
            }
        }
    }

    template<typename calc=short,typename IType=fb,typename FType=short>
    constexpr inline void circle(const IType &cx, const IType &cy, const FType &r, const pixel &px, const bool fill=true) {
        const FType r2 = r * r;

        for (IType x = cx - r; x < cx + r + 1; x++) {
            for (IType y = cy - r; y < cy + r + 1; y++) {
                const calc xy2 = (calc(x-cx) * calc(x-cx)) + (calc(y-cy) * calc(y-cy));
                const calc xyd = r2 - xy2;
                if (!fill && (xyd >= 0 ? xyd : -xyd) > (r*2))
                    continue;
                if (xy2 < r2)
                    this->putPixel(x, y, px);
            }
        }
    }

    constexpr inline void putPixelBound(const fb &x, const fb &y, const pixel &px) {
        if (this->isBound(x, y))
            this->putPixel(x, y, px);
    }

    template<typename T>
    constexpr inline void putTexture(const T &texture, const fb &dx, const fb &dy, const fb &w = 0, const fb &h = 0, const fb &sx = 0, const fb &sy = 0) {
        const fb width = w ? w : texture.getWidth();
        const fb height = h ? h : texture.getHeight();

        for (fb i = 0, k = dx; i < w && k < width; i++, k++) {
            for (fb j = 0, l = dy; j < h && l < height; j++, l++) {
                pixel srcPix = texture.getPixel(i + sx, j + sy);
                if (texture.BPP > 1) {
                    //if (!(srcPix & 1))
                    //if (~srcPix & 1)
                    //    continue;
                    //srcPix >>= (texture.bpp - this->bpp);
                    srcPix = ((srcPix & 1) && srcPix > 1) ? 1 : 0;
                }
                this->putPixel(k, l, srcPix);
            }
        }
    }

    template<typename T>
    constexpr inline void putTexture(const T *texture, const fb &dx, const fb &dy, const fb &w = 0, const fb &h = 0, const fb &sx = 0, const fb &sy = 0) {
        putTexture(texture, {dx, dy, w, h}, {sx, sy});
    }

    template<typename T>
    constexpr inline void putTexture(const T *texture, const Size &texture_size, const Origin &position) {
        const Length tll = texture->getLength();

        const fb tr = texture_size.getRight() <= tll.width ? texture_size.getRight() : tll.width;
        const fb tb = texture_size.getBottom() <= tll.height ? texture_size.getBottom() : tll.height;
        const fb tl = texture_size.getLeft();
        const fb tt = texture_size.getTop();
        const fb alpha = texture->getAlphaTest();
        const fb dest = this->getValueBits();

        const Length dll = this->getLength();

        const fb dr = dll.width;
        const fb db = dll.height;
        const fb dl = position.x;
        const fb dt = position.y;

        for (fb dx = dl, tx = tl; dx < dr && tx < tr; dx++, tx++) {
            for (fb dy = dt, ty = tt; dy < db && ty < tb; dy++, ty++) {
                const pixel px = texture->getPixel(tx, ty);
                if (px > alpha)
                    this->putPixel(dx, dy, dest);
            }
        }
    }

    template<typename T>
    constexpr inline void putTexture(const T &texture, const Size &texture_size, const Origin &position) {
        putTexture(&texture, texture_size, position);
    }

    template<typename _SpriteT = Sprite>
    constexpr inline void putSprite(const _SpriteT &sprite, const fb &x, const fb &y, const fb &w = 0, const fb &h = 0) {
        putTexture(sprite.src, sprite, {x,y});
        /*
        const fb width = w ? w : sprite.getWidth();
        const fb height = h ? h : sprite.getHeight();

        for (fb i = 0, k = x; i < width; i++, k++) {
            for (fb j = 0, l = y; j < height; j++, l++) {
                pixel srcPix = sprite.src.getPixel(i + sprite.x0, j + sprite.y0);
                if (sprite.src.BPP > 1) {
                    srcPix = ((srcPix & 1) && srcPix > 1) ? 1 : 0;
                    //if (~(srcPix & 1))
                    //    srcPix = 0;
                    //srcPix >>= (sprite.src.bpp - this->bpp);
                }
                this->putPixel(k, l, srcPix);
            }
        }
        */
    }

    template<typename _SpriteT = Sprite>
    constexpr inline void putSprite(const _SpriteT &sprite, const Origin &position) {
        putTexture(sprite.src, sprite, position);
    }

    template<typename calc=short, typename CALLBACK>
    constexpr inline void line_callback(const fb &x1, const fb &y1, const fb &x2, const fb &y2, CALLBACK callback) {
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

    constexpr inline void line(const fb &x1, const fb &y1, const fb &x2, const fb &y2, const pixel &px) {
        line_callback(x1, y1, x2, y2, 
            [this, px](const fb &x, const fb &y) { this->putPixelBound(x, y, px); }
        );
    }

    constexpr inline void line(const Origin &start, const Origin &end, const pixel &px) {
        line(start.x, start.y, end.x, end.y, px);
    }

    template<typename calc=short, typename IType=fb, typename FType=fb, typename CALLBACK>
    constexpr inline void stroke_line_callback(const IType &x1, const IType &y1, const IType &x2, const IType &y2, const FType &width, CALLBACK callback) { 
        line_callback<calc>(x1, y1, x2, y2, [this,&width,&callback](const IType &x, const IType &y) {
            this->circle_callback<calc,IType,FType>(callback, x, y, width, true);
        });
    }

    template<typename calc=short, typename IType=fb, typename FType=fb>
    constexpr inline void stroke_line(const IType &x1, const IType &y1, const IType &x2, const IType &y2, const FType &width, const pixel &px) {
        stroke_line_callback<calc,IType,FType>(x1, y1, x2, y2, width, [this, &px](const IType &x, const IType &y) {
            this->putPixelBound(x, y, px);
        });
    }
};

template<typename Texture>
struct TextureGraphicsContext : public Texture, public IGraphicsContext {
    using Texture::Texture;

    inline void fill(const Size &size, const pixel &px) override {
        Texture::fill(size, px);
    }
    
    inline void putPixel(const Origin &pos, const pixel &px) override {
        Texture::putPixel(pos, px);
    }

    inline void putPixel(const fb &x, const fb &y, const pixel &px) override {
	Texture::putPixel(x, y, px);
    }

    inline pixel getPixel(const fb &x, const fb &y) const override {
	return Texture::getPixel(x, y);
    }

    inline pixel getPixel(const Origin &pos) const override {
        return Texture::getPixel(pos);
    }

    inline void putTexture(const IGraphicsContext *texture, const Size &texture_size, const Origin &destination_origin) override {
        Texture::putTexture(texture, texture_size, destination_origin);
    }

    inline void putSprite(const Sprite &sprite, const Origin &position) override {
        Texture::putSprite(sprite, position);
    }

    inline void circle(const Origin &center, const fb &radius, const pixel &px, const bool &fill = true) override {
        Texture::circle(center, radius, px, fill);
    }

    inline bool isBound(const Origin &pos) const override {
        return Texture::isBound(pos);
    }

    inline Length getLength() const override {
        return Length(Texture::getWidth(), Texture::getHeight());
    }

    inline void line(const Origin &pos_start, const Origin &pos_end, const pixel &px) {
        return Texture::line(pos_start, pos_end, px);
    }

    inline fb getAlphaTest() const override {
        return Texture::getAlphaTest();
    }
};

template<typename Buffer, typename Derived = TextureT<Buffer>, typename SpriteT = SpriteT<Derived>>
struct AtlasT : public Derived {
    using Derived::Derived;
    using Sprite = SpriteT;

    inline constexpr Sprite getSprite(const fb &x, const fb &y, const fb &sx, const fb &sy) const {
        return Sprite(this, x, y, /*x+*/sx, /*y+*/sy);
    }

    inline constexpr Sprite getSpriteAligned(const fb &x, const fb &y, const fb &nx, const fb &ny, const fb &sw, const fb &sh) const {
        return getSprite(x * sw, y * sh, nx * sw, ny * sh);
    }
};

using DisplayTexture = TextureT<DisplayBuffer>;

DisplayTexture display;

}
