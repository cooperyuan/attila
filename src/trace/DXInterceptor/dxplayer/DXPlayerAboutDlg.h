////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerAboutDlg : public CDialog
{
public:

  enum { IDD = IDD_ABOUTBOX };

  DXPlayerAboutDlg();

protected:

  virtual void DoDataExchange(CDataExchange* pDX);
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////
