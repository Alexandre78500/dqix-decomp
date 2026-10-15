#pragma once

extern "C"
{
    void GX_LoadBGPltt(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBGPltt(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadOBJPltt(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadOBJPltt(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadOAM(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadOAM(const void* data, unsigned int offset, unsigned int length);

    void GX_LoadOBJ(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadOBJ(const void* data, unsigned int offset, unsigned int length);

    void GX_LoadBG0Scr(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG0Scr(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG1Scr(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG1Scr(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG2Scr(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG2Scr(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG3Scr(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG3Scr(const void* data, unsigned int offset, unsigned int length);

    void GX_LoadBG0Char(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG0Char(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG1Char(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG1Char(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG2Char(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG2Char(const void* data, unsigned int offset, unsigned int length);
    void GX_LoadBG3Char(const void* data, unsigned int offset, unsigned int length);
    void GXS_LoadBG3Char(const void* data, unsigned int offset, unsigned int length);

    void GX_BeginLoadBGExtPltt();
    void GX_LoadBGExtPltt(const void* data, unsigned int offset, unsigned int length);
    void GX_EndLoadBGExtPltt();

    void GX_BeginLoadOBJExtPltt();
    void GX_LoadOBJExtPltt(const void* data, unsigned int offset, unsigned int length);
    void GX_EndLoadOBJExtPltt();

    void GXS_BeginLoadBGExtPltt();
    void GXS_LoadBGExtPltt(const void* data, unsigned int offset, unsigned int length);
    void GXS_EndLoadBGExtPltt();

    void GXS_BeginLoadOBJExtPltt();
    void GXS_LoadOBJExtPltt(const void* data, unsigned int offset, unsigned int length);
    void GXS_EndLoadOBJExtPltt();

    void GX_BeginLoadTex();
    void GX_LoadTex(const void* data, unsigned int offset, unsigned int length);
    void GX_EndLoadTex();

    void GX_BeginLoadTexPltt();
    void GX_LoadTexPltt(const void* data, unsigned int offset, unsigned int length);
    void GX_EndLoadTexPltt();

    void GX_BeginLoadClearImage();
    void GX_LoadClearImageColor(const void* data, unsigned int length);
    void GX_LoadClearImageDepth(const void* data, unsigned int length);
    void GX_EndLoadClearImage();
}