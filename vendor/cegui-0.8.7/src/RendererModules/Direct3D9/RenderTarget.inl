/***********************************************************************
    created:    Tue Feb 10 2009
    author:     Paul D Turner
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2011 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#include "CEGUI/RendererModules/Direct3D9/RenderTarget.h"
#include "CEGUI/RendererModules/Direct3D9/GeometryBuffer.h"
#include "CEGUI/RenderQueue.h"
#include "CEGUI/Exceptions.h"
#include <d3dx9.h>

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
template <typename T>
Direct3D9RenderTarget<T>::Direct3D9RenderTarget(Direct3D9Renderer& owner) :
    d_owner(owner),
    d_device(owner.getDevice()),
    d_area(0, 0, 0, 0),
    d_matrixValid(false),
    d_viewDistance(0)
{
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::draw(const GeometryBuffer& buffer)
{
    buffer.draw();
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::draw(const RenderQueue& queue)
{
    queue.draw();
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::setArea(const Rectf& area)
{
    d_area = area;
    d_matrixValid = false;

    RenderTargetEventArgs args(this);
    T::fireEvent(RenderTarget::EventAreaChanged, args);
}

//----------------------------------------------------------------------------//
template <typename T>
const Rectf& Direct3D9RenderTarget<T>::getArea() const
{
    return d_area;
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::activate()
{
    if (!d_matrixValid)
        updateMatrix();

    D3DVIEWPORT9 vp;
    setupViewport(vp);
    d_device->SetViewport(&vp);

    d_owner.getDevice()->SetTransform(D3DTS_PROJECTION, &d_matrix);
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::deactivate()
{
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::unprojectPoint(const GeometryBuffer& buff,
                                             const Vector2f& p_in,
                                             Vector2f& p_out) const
{
    if (!d_matrixValid)
        updateMatrix();

    const Direct3D9GeometryBuffer& gb =
        static_cast<const Direct3D9GeometryBuffer&>(buff);

    D3DVIEWPORT9 vp;
    setupViewport(vp);

    D3DXVECTOR3 in(p_in.d_x, p_in.d_y, 0.0f);
    D3DXVECTOR3 out;
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    D3DXVec3Unproject(&out, &in, &vp, &d_matrix, &identity, gb.getMatrix());

    p_out.d_x = out.x;
    p_out.d_y = out.y;
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::updateMatrix() const
{
    const float w = d_area.getWidth();
    const float h = d_area.getHeight();

    // We need this because we need to adapt to D3D's texel positioning: geometry must move half a pixel
    // left and up. The view matrix below flips y (_22 = -1), so moving up needs a positive y offset; with
    // -0.5 every pixel sampled halfway between two texture rows, which hid the last row of images (e.g. the
    // bottom border line of frames and buttons).
    const float x_offset = -0.5f;
    const float y_offset = 0.5f;

    // set up projection parameters
    const float fov = 0.523598776f; // 30 degrees
    const float aspect = (h != 0.0f) ? (w / h) : 1.0f;
    const float half_fov = fov * 0.5f;
    d_viewDistance = (aspect != 0.0f) ? ((w / aspect) / (2.0f * tanf(half_fov))) : 1000.0f;

    // set up projection matrix with far plane safely beyond view distance
    const float pnear = 0.5f;
    const float pfar = ceguimax(10000.0f, d_viewDistance * 3.0f);

    D3DXMatrixPerspectiveFovLH(&d_matrix, fov, aspect, pnear, pfar);

    D3DXMATRIX view;
    D3DXMatrixIdentity(&view);
    view._22 = -1.0f;
    view._41 = -w * 0.5f + x_offset;
    view._42 = h * 0.5f + y_offset;
    view._43 = d_viewDistance;

    D3DXMatrixMultiply(&d_matrix, &view, &d_matrix);

    d_matrixValid = true;
}

//----------------------------------------------------------------------------//
template <typename T>
void Direct3D9RenderTarget<T>::setupViewport(D3DVIEWPORT9& vp) const
{
    vp.X = static_cast<DWORD>(d_area.left());
    vp.Y = static_cast<DWORD>(d_area.top());
    vp.Width = static_cast<DWORD>(d_area.getWidth());
    vp.Height = static_cast<DWORD>(d_area.getHeight());
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section
