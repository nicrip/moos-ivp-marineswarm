function values = readmseaspe_moossafir (strvar,lon,lat,depth,time);
%
% readmseaspe was written for Matlab by Patrick J. Haley, Jr. (MIT MSEAS Group)
%
% this script, readmseaspe_moossafir, has been minimally modified for Octave from
% readmseaspe, and is different to readmseaspe_octave, in that it allows batch
% (lon, lat, etc. arguments as vectors) arguments and successfully returns. In
% addition, it makes use if interp1_alt.m instead of the original interp1, an
% altenative interpolation script written by Paul Kienzle, which provides a
% significant speedup. Finally, all clear calls are removed except for the final
% clear all, for an additional speedup; however this can cause segfaults when
% octave closes unexpectedly.
%
% these modifications have been made by;
% Nick R. Rypkema (rypkema@mit.edu, MIT LAMSS Group)
%
% for additional information on the MSEAS Primitive Equation dynamical model
% see the publication Haley, P.J., Jr. and P.F.J. Lermusiaux, 2010. Multiscale
% two-way embedding schemes for free-surface primitive-equations in the
% Multidisciplinary Simulation, Estimation and Assimilation System.
% Ocean Dynamics, 60, 1497-1537. doi:10.1007/s10236-010-0349-4.
%
% values = readmseaspe_moossafir (strvar,lon,lat,depth,time);
%
% This function manages the reading of a MSEAS PE file or several
% related MSEAS PE files.
%
% ------
% Input:
% ------
%
%    strvar...Usually the string name of the field to read
%             or a cell array of such strings.  On the first
%             call, the name(s) of the MSEAS PE file(s) to read.
%    lon......The longitude(s) at which the values are sought.
%             (positive East)
%             On the first call, lon can, optionally, be a scalar
%             flag value indicating the allowed vertical extrapolations.
%                [0] no extrapolations.
%                [1] only extrapolate up (towards the surface).
%                [2] extrapolate in both directions.  [default]
%    lat......The latitude(s) at which the values are sought.
%             (positive North)
%    depth....The depth(s) at which the values are sought.
%             (positive down)
%    time.....The times [year month day hour minute second]
%             at which the values are sought.
%
% -------
% Output:
% -------
%
%    values...The values interpolated from the MSEAS PE.
%
% ------
% Usage:
% ------
%
% Overview:
% ---------
%
% This collection of functions is designed to manage the reading and
% interpolation of MSEAS PE output in Matlab with most details hidden
% from the user.  There are 3 basic steps:
%
%    (1) Initialization.  A single call opens the PE file(s) and
%        creates local, static variables.
%
%    (2) Data Extraction.  Subsequent calls read the PE file(s) and
%        interpolate the fields to the requested positions.  These
%        calls may be repeated as many times as the user desires.
%
%    (3) Clean Up.  A single call closes the PE file(s) and clears
%        the local, static variables.
%
% This functions are designed to work in multiple configurations:
%
%    (1) All data in a single domain and in a single file.
%    (2) All data in a single domain broken up into 2 files
%        for size reasons
%    (3) Data from a set of telescoping nested domains, with
%        each domain contained in a single file.
%    (4) Data from a set of telescoping nested domains, with
%        each domain broken up into 2 files.
%
% Initialization:
% ---------------
%
% The first call to readmseaspe is always just to open the necessary PE
% netCDF file(s).  Generally, only a single argument is passed in, the
% file name(s).
%
% If a single PE file contains all the data, just pass that name as a
% string array:
%
%    readmseaspe('pefile.nc');
%
% If the information is broken up into 2 files with one containing the
% tracer output and the second containing the corresponding velocity
% output (same times and grids), pass these in as a cell array of strings
% with the tracer file in the first position:
%
%    readmseaspe({'Tpefile.nc' 'Vpefile.nc'});
%
% If the information comes from a set of L telescoping nested domain, each
% of which is contained in a single file, the argument is a cell array of
% length 1 containing a cell array of length L of file names:
%
%    readmseaspe({{'smallest_pefile.nc' ... 'largest_pefile.nc'}});
%
% If the information comes from a set of L telescoping nested domain, each
% of which is broken up into 2 files with one containing the tracer output
% and the second containing the corresponding velocity output, the argument
% is a cell array of length 2 containing cell arrays of length L of file names:
%
%    readmseaspe({{'smallest_Tpefile.nc' ... 'largest_Tpefile.nc'} ...
%                {'smallest_Vpefile.nc' ... 'largest_Vpefile.nc'}});
%
% Note that if you wish to limit the vertical extrapolations, a second argument
% can be given with any of the above forms (see description of allowed
% values in above documentation of lon):
%
%    if ~isempty( readmseaspe({{'smallest_Tpe.nc' ... 'largest_Tpe.nc'} ...
%                             {'smallest_Vpe.nc' ... 'largest_Vpe'}}, 1) )
%       error ('Unable to successfully open MSEAS PE file.');
%    end;
%
% If the file(s) were successfully opened, readmseaspe returns the empty set
% ([]), otherwise it returns -1.
%
% Data Extraction:
% ----------------
%
% On subsequent calls, readmseaspe interpolates the requested fields at
% the requested positions.  The fields are requested in the first
% argument (STRVAR).  This can be a string for a single field or a cell
% array of length (M) of strings for M different fields.  Valid names are
%
%    't' 'temp' 'temperature'
%    's' 'salt' 'salinity'
%    'c' 'sound speed' 'sound velocity'
%    'w' 'vertical velocity'
%    'u' 'zonal velocity'
%    'v' 'meridional velocity'
%
% For each requested field, readmseaspe interpolates the PE netCDF file(s)
% to the requested positions specified in LON, LAT, DEPTH, TIME where
% LON, LAT and DEPTH are vectors of length N and TIME is a matrix of
% size (N,6).  The output of readmseaspe is a cell array of length M whose
% elements are vectors of length N.
%
% There are 2 kinds of errors that can occur:
%
%    (1) An invalid field is requested.  In this case, the corresponding
%        element of the cell array is the empty set [].
%    (2) An out-of-range position is requested.  This can occur for a point
%        outside all domains, a time outside the range of the PE files or
%        a point under the land mask.  In any of these cases, the corresponding
%        value of the output vector(s) is a NaN.
%
% Clean Up:
% ---------
%
% When complete finished reading the file(s), call readmseaspe with no
% arguments to close the files and clear its internal storage.  If the file(s)
% were successfully closed, readmseaspe returns the empty set ([]), otherwise it
% returns -1.
%
%    if ~isempty(readmseaspe)
%       error ('Unable to successfully close MSEAS PE file.');
%    end;
%
% In the above example, the output of the "clean-up" call is directly tested
% and the calling program aborts on error.

warning('off', 'Octave:possible-matlab-short-circuit-operator');
do_braindead_shortcircuit_evaluation(1);

%-------------------------------------------------------------------------------
% Set internal static storage.
%-------------------------------------------------------------------------------

persistent ncids fnames domdat lndinds timdat xtrflg rotflag

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

values = [];

%-------------------------------------------------------------------------------
% On first call, open file(s) and initialize data.
%-------------------------------------------------------------------------------

if ((nargin==1) | (nargin==2))

   [ncids,fnames,domdat,lndinds,timdat,rotflag] = initmseas (strvar);
   if isempty(rotflag)
      values = -1;
   else
      values = ncids;
   end;

   if (nargin==1)
      xtrflg = 2;
   else
      xtrflg = lon;
   end;

   return;

end;

%-------------------------------------------------------------------------------
% On last call, close file(s) and clean-up.
%-------------------------------------------------------------------------------

if (nargin==0)

   values = closmseas (ncids,fnames);

   %clear ncids fnames domdat lndinds timdat xtrflg rotflag;
   clear all;
   disp('Cleared all internal && persistent variables.')

   return;

end;

%-------------------------------------------------------------------------------
% Eliminate some error conditions.
%-------------------------------------------------------------------------------

if (nargin~=5)
   disp (' ');
   disp (['***Error:  READMSEAS - invalid number (',num2str(nargin), ...
          ') of input arguments.']);
   disp ('           Valid numbers of input arguments are [0, 1, 5]');
   disp (' ');
   return;
end;

if isempty(timdat)
   disp (' ');
   disp ('***Error:  READMSEAS - input file was not successfully opened.');
   disp (' ');
   return;
end;

%-------------------------------------------------------------------------------
% Convert position data.
%-------------------------------------------------------------------------------

[domt,xt,it,yt,jt,zt,tt,domv,xv,iv,yv,jv,zv,tv] = getpepos (lon,lat,depth, ...
                                                            time,domdat,timdat);

%-------------------------------------------------------------------------------
% Interpolate requested fields.
%-------------------------------------------------------------------------------

if iscell(strvar)
   varnames = strvar;
else
   varnames = {strvar};
end;

for nvar = 1:length(varnames)

   switch upper(varnames{nvar});
      case {'T' 'TEMP' 'TEMPERATURE'}
         values{nvar} = intrppe ('temp',ncids{1},fnames{1},[],domt, ...
                                 it,xt,jt,yt,zt,tt,squeeze(domdat(:,3)), ...
                                 lndinds{1},xtrflg);
      case {'S' 'SALT' 'SALINITY'}
         values{nvar} = intrppe ('salt',ncids{1},fnames{1},[],domt, ...
                                 it,xt,jt,yt,zt,tt,squeeze(domdat(:,3)), ...
                                 lndinds{1},xtrflg);
      case {'C' 'SOUND SPEED' 'SOUND VELOCITY'}
         values{nvar} = intrppe ('sndsp',ncids{1},fnames{1},[],domt, ...
                                 it,xt,jt,yt,zt,tt,squeeze(domdat(:,3)), ...
                                 lndinds{1},xtrflg);
      case {'W' 'VERTICAL VELOCITY'}
         values{nvar} = intrppe ('wvzv',ncids{2},fnames{2},[],domv, ...
                                 iv,xv,jv,yv,zv,tv,squeeze(domdat(:,3)), ...
                                 lndinds{2},xtrflg);
      case {'U' 'ZONAL VELOCITY'}
         values{nvar} = intrppe ('vtot',ncids{2},fnames{2},1,domv, ...
                                 iv,xv,jv,yv,zv,tv,squeeze(domdat(:,3)), ...
                                 lndinds{2},xtrflg);
%         if ~rotflag
%            disp('  extracting u-velocity: oriented along model grid');
%         else
%            disp('  extracting u-velocity: true East-West orientation');
%         end;
      case {'V' 'MERIDIONAL VELOCITY'}
         values{nvar} = intrppe ('vtot',ncids{2},fnames{2},2,domv, ...
                                 iv,xv,jv,yv,zv,tv,squeeze(domdat(:,3)), ...
                                 lndinds{2},xtrflg);
%         if ~rotflag
%            disp('  extracting v-velocity: oriented along model grid');
%         else
%            disp('  extracting v-velocity: true North-South orientation');
%         end;
      otherwise
         values{nvar} = [];
   end;

end;

return;

%===============================================================================

function [ncids,fnames,domdat,lndinds,timdat,rotflag] = initmseas (strvar);
%
% [ncids,fnames,domdat,lndinds,timdat,rotflag] = initmseas (strvar);
%
% This function opens the MSEAS PE netCDF file(s) and reads initial data.
%
% ------
% Input:
% ------
%
%    strvar....Either name of MSEAS PE netCDF file or cell array
%              of names of 2 MSEAS PE netCDF files.
%
% -------
% Output:
% -------
%
%    ncids.....Cell array of netCDF identifiers to open files.
%                NCIDS{1}  -  netCDF identifer for tracer file.
%                NCIDS{2}  -  netCDF identifer for velocity file.
%                             Can be same as tracer file.
%    fnames....Cell array of input file names.
%                FNAMES{1}  -  name of tracer file.
%                FNAMES{2}  -  name of velocity file.
%                              Can be same as tracer file.
%    domdat....Vector of domain definition data.
%                DOMDAT( 1)  -  COORD.  Coordinate definition.
%                DOMDAT( 2)  -  NX.     Number grid points in X-direction.
%                DOMDAT( 3)  -  NY.     Number grid points in Y-direction.
%                DOMDAT( 4)  -  DX.     Grid spacing in X-direction.
%                DOMDAT( 5)  -  DY.     Grid spacing in Y-direction.
%                DOMDAT( 6)  -  LONC.   Longitude of transformation center.
%                DOMDAT( 7)  -  LATC.   Latitude of transformation center.
%                DOMDAT( 8)  -  DELX.   Transformation-Grid offset, X-direction.
%                DOMDAT( 9)  -  DELY.   Transformation-Grid offset, Y-direction.
%                DOMDAT(10)  -  THETA.  Rotation angle.
%    lndinds...Cell array of arrays of land indices.
%                LNDINDS(1)  -  TLNDIND.  Land indices on tracer grid.
%                LNDINDS(2)  -  VLNDIND.  Land indices on velocity grid.
%    timdat....Array to time data.
%                TIMDAT(1)  -  TIMEOFF.    Initialzation time offset for datenum.
%                TIMDAT(2)  -  TIMEND.     Last day of PE output.
%                TIMDAT(3)  -  TIMESCALE.  Scaling from day to PE/PI time.
%                TIMDAT(4)  -  TIME0.      First day of PE output.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

ncids   = [];
fnames  = [];
domdat  = [];
lndinds = [];
timdat  = [];

%-------------------------------------------------------------------------------
% Open file(s).
%-------------------------------------------------------------------------------

if ischar(strvar)

   %---------------------------
   %--- Single netCDF file. ---
   %---------------------------

   tfile{1} = strvar;

   nctid{1} = netcdf(tfile{1},'r');
   if isnull(nctid{1})
      disp (' ');
      disp ('In loop for single netcdf file');
      disp ('***Error:  INITMSEAS - unable to open netCDF file:');
      disp (['           ',setstr(34),tfile{1},setstr(34)]);
      disp (' ');
      return;
   end;

   vfile = tfile;
   ncvid = nctid;

elseif isstr(strvar{1})

   %-------------------------
   %--- Two netCDF files. ---
   %-------------------------

   tfile{1} = strvar{1};
   vfile{1} = strvar{2};

   nctid{1} = netcdf(tfile{1},'r');
   if isnull(nctid{1})
      disp (' ');
      disp ('In T loop for two netcdf files');
      disp ('***Error:  INITMSEAS - unable to open netCDF file:');
      disp (['           ',setstr(34),tfile{1},setstr(34)]);
      disp (' ');
      return;
   end;

   ncvid{1} = netcdf(vfile{1},'r');
   if isnull(ncvid{1})
      disp (' ');
      disp ('In V loop for two netcdf files');
      disp ('***Error:  INITMSEAS - unable to open netCDF file:');
      disp (['           ',setstr(34),vfile{1},setstr(34)]);
      disp (' ');
      return;
   end;

elseif (length(strvar)==1)

   %---------------------------------------
   %--- Single chain of nested domains. ---
   %---------------------------------------

   ndoms = length(strvar{1});
   tfile = strvar{1};

   for n = 1:ndoms
      nctid{n} = netcdf(tfile{n},'r');
      if isnull(nctid{n})
         disp (' ');
         disp ('***Error:  INITMSEAS - unable to open netCDF file:');
         disp (['           ',setstr(34),tfile{n},setstr(34)]);
         disp (' ');
         return;
      end;
   end;

   vfile = tfile;
   ncvid = nctid;

else

   %---------------------------------------
   %--- Double chain of nested domains. ---
   %---------------------------------------

   ndoms = length(strvar{1});
   tfile = strvar{1};
   vfile = strvar{2};

   for n = 1:ndoms
      nctid{n} = netcdf(tfile{n},'r');
      if isnull(nctid{n})
         disp (' ');
         disp ('***Error:  INITMSEAS - unable to open netCDF file:');
         disp (['           ',setstr(34),tfile{n},setstr(34)]);
         disp (' ');
         return;
      end;

      if ~isnull(vfile{n})
         ncvid{n} = netcdf(vfile{n},'r');
         if isnull(ncvid{n})
            disp (' ');
            disp ('***Error:  INITMSEAS - unable to open netCDF file:');
            disp (['           ',setstr(34),vfile{n},setstr(34)]);
            disp (' ');
            return;
         end;
      else
         ncvid{n} = nctid{n};
         vfile{n} = tfile{n};
      end
   end;

end;

ncids  = {nctid ncvid};
fnames = {tfile vfile};

%-------------------------------------------------------------------------------
% Read domain definition parameters.
%-------------------------------------------------------------------------------

ndom = length(fnames{1});

domdat = NaN.*ones(ndom,10);
timdat = NaN.*ones(ndom,4);

for n = 1:ndom
   [domdatn,tlndindn,vlndindn,timdatn] =  ...
                                getpedom (nctid{n},tfile{n},ncvid{n},vfile{n});
   domdat(n,:) = domdatn;
   tlndind{n}  = tlndindn;
   vlndind{n}  = vlndindn;
   timdat(n,:) = timdatn;
end;

lndinds = {tlndind vlndind};

rotflag = ~isempty(findstr(vfile{1},'_geo')) | ...
          ~isempty(findstr(ncvid{1}.history(:),'vtot rotated'));

%clear nctid ncvid ndom tfile vfile domdatn tlndindn vlndindn timdatn tlndind vlndind

return;

%===============================================================================

function [domdat,tlndind,vlndind,timdat] = getpedom (nctid,tfile,ncvid,vfile);
%
% [domdat,tlndind,vlndind,timdat] = getpedom (nctid,tfile,ncvid,vfile);
%
% This runction reads the spatial/temporal domain parameters from a
% MSEAS PE netCDF file.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

domdat  = [];
tlndind = [];
vlndind = [];
timdat  = [];

%-------------------------------------------------------------------------------
% Set useful constants.
%-------------------------------------------------------------------------------

cm2m = 0.01;
cm2deg = 180.0/(pi*637131500.0);

%-------------------------------------------------------------------------------
% Determine type of PE output (PE model or PE_initial).
%-------------------------------------------------------------------------------

piflag = strcmp(upper(nctid.type(:)),'PE_INITIAL');

%-------------------------------------------------------------------------------
% Determine vintage of PE model.
%-------------------------------------------------------------------------------

if ~piflag
   %------------------------
   %--- Read PE version. ---
   %------------------------

   peversion = nctid.version(:);

   %-----------------------------------------------------
   %--- Convert newer strings to older numeric forms. ---
   %-----------------------------------------------------

   if ischar(peversion)
      pstr = peversion;
      rmind = find (isletter(pstr) | isspace(pstr));
      if ~isempty(rmind)
         pstr(rmind) = [];
      end;
      peversion = str2num(pstr);
      %clear pstr rmind;
   end;

   %-----------------------------------------------------------
   %--- Determine version and release.  Technically the     ---
   %--- logic for the release doesn't work since it equates ---
   %--- the releases for, say, 7.7 and 7.70.                ---
   %-----------------------------------------------------------

   pever = fix(peversion);
   perel = round((peversion-pever)*100);

   %clear peversion;

   %-----------------------------------------------------------
   %--- Determine vintage.  Correct for release logic flaw. ---
   %-----------------------------------------------------------

   oldpe = ((pever<7) | ((pever==7) & (perel~=28)));

   %clear pever perel;

else

   oldpe = 0;

end;

%-------------------------------------------------------------------------------
% Read domain definition parameters.
%-------------------------------------------------------------------------------

%-----------------------------
%--- Coordinate type flag. ---
%-----------------------------

name  = 'coord';
coord = nctid{name}(:);
if (isempty(coord) & isempty(nctid{name}))
   coord = 0;
elseif isempty(coord)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

%--------------------
%--- Domain size. ---
%--------------------

name  = 'tlon';
nx    = length(nctid(name));
if isempty(nx)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

name  = 'tlat';
ny    = length(nctid(name));
if isempty(ny)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

%---------------------
%--- Grid Spacing. ---
%---------------------

if ~piflag
   name  = 'dxt';
   dx    = nctid{name}(1);
   if isempty(dx)
      disp (' ');
      disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
             setstr(34),' in netCDF file:']);
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;

   name  = 'dyt';
   dy    = nctid{name}(1);
   if isempty(dy)
      disp (' ');
      disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
             setstr(34),' in netCDF file:']);
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;

   if (coord~=0)
      dx = dx.*cm2deg;
      dy = dy.*cm2deg;
   else
      dx = dx.*cm2m;
      dy = dy.*cm2m;
   end;

else
   name  = 'gridx';
   dx    = nctid{name}(1);
   if isempty(dx)
      disp (' ');
      disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
             setstr(34),' in netCDF file:']);
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;

   name  = 'gridy';
   dy    = nctid{name}(1);
   if isempty(dy)
      disp (' ');
      disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
             setstr(34),' in netCDF file:']);
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;

   if (coord==0)
      dx = dx.*cm2m;
      dy = dy.*cm2m;
   end;
end;

%------------------------------
%--- Transformation Center. ---
%------------------------------

name  = 'rlngd';
lonc  = nctid{name}(:);
if isempty(lonc)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

name  = 'rlatd';
latc  = nctid{name}(:);
if isempty(latc)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

%-----------------------------------
%--- Transformation-Grid Offset. ---
%-----------------------------------

name  = 'delx';
delx  = nctid{name}(:);
if (isempty(delx) & isempty(nctid{name}) & (coord==0))
   delx = 0;
elseif isempty(delx)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

name  = 'dely';
dely  = nctid{name}(:);
if (isempty(dely) & isempty(nctid{name}) & (coord==0))
   delx = 0;
elseif isempty(dely)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

if (coord==0)
   delx = delx.*cm2m;
   dely = dely.*cm2m;
end;

%-----------------
%--- Rotation. ---
%-----------------

name  = 'thetad';
theta = nctid{name}(:);
if isempty(theta)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

%------------------------
%--- Old PE versions. ---
%------------------------

if (oldpe)
   deg2rad = pi/180;
   dx      = dx*cos(latc*deg2rad)
   %clear deg2rad
end;

%clear oldpe

%-------------------------
%--- Group parameters. ---
%-------------------------

domdat = [coord nx ny dx dy lonc latc delx dely theta];

%clear coord nx ny dx dy lonc latc delx dely theta;

%-------------------------------------------------------------------------------
% Read land masks.
%-------------------------------------------------------------------------------

name    = 'landt';
try
   landt  = nctid{name}(:);
catch
   disp ('landt variable does not exist in this file')
   landt = [];
end_try_catch
if ~isempty(landt)
   tlndind = find (landt==0);
elseif isempty(landt)
   tlndind = [];
else
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

name    = 'landv';
try
   landv  = nctid{name}(:);
catch
   disp ('landt variable does not exist in this file')
   landv = [];
end_try_catch
if ~isempty(landv)
   vlndind = find (landv~=2);
elseif isempty(landv)
   vlndind = [];
else
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),vfile,setstr(34)]);
   disp (' ');
   return;
end;

%clear landt landv;

%-------------------------------------------------------------------------------
% Read time limitations.
%-------------------------------------------------------------------------------

if ~piflag
   timeoff = datenum(get_petim0(nctid));
   if isempty(timeoff)
      disp (' ');
      disp ('***Error:  GETPEDOM - unable to read initial time in netCDF file:');
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;
   timescale = 86400;
   t0 = 0;
else
   name    = 'time';
   jdoff   = nctid{name}.add_offset(:);
   if isempty(jdoff)
      disp (' ');
      disp ('***Error:  GETPEDOM - unable to read initial time in netCDF file:');
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;
   t0      = nctid{name}(1);
   if isempty(t0)
      disp (' ');
      disp ('***Error:  GETPEDOM - unable to read initial time in netCDF file:');
      disp (['           ',setstr(34),tfile,setstr(34)]);
      disp (' ');
      return;
   end;
   timeoff = datenum(gregorian(t0+jdoff)) - t0; % a subtractive not additive factor.
   %clear jdoff
   timescale = 1;
end;

name    = 'time';
timend  = nctid{name}(length(nctid(name)));
if isempty(timend)
   disp (' ');
   disp (['***Error:  GETPEDOM - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),tfile,setstr(34)]);
   disp (' ');
   return;
end;

timdat = [timeoff timend timescale t0];

%clear nctid ncvid tfile vfile timeoff timend timescale t0 name;

return;

%===============================================================================

function status = closmseas (ncids,fnames);
%
% status = closmseas (ncids,fnames);
%
% This function closes the MSEAS PE netCDF files.
%
% ------
% Input:
% ------
%
%    ncids....Cell array of netCDF identifiers to open files.
%                NCIDS{1}  -  netCDF identifer(s) for tracer file(s).
%                NCIDS{2}  -  netCDF identifer(s) for velocity file(s).
%                             Can be same as tracer file.
%    fnames...Cell array of input file names.
%                FNAMES{1}  -  name(s) of tracer file(s).
%                FNAMES{2}  -  name(s) of velocity file(s).
%                              Can be same as tracer file(s).
%
% -------
% Output:
% -------
%
%    status...Exit status.  Empty if successful, -1 if not.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

status = [];

%-------------------------------------------------------------------------------
% Loop over all nested domains, closing files.
%-------------------------------------------------------------------------------

ncidt = ncids{1};
tfile = fnames{1};

ncidv = ncids{2};
vfile = fnames{2};

for n = 1:length(tfile)
   stw = clos1mseas ({ncidt{n} ncidv{n}},{tfile{n} vfile{n}});
   status = [status stw];
end;

%clear ncidt ncidv tfile vfile n stw

%-------------------------------------------------------------------------------
% Set final exit status.
%-------------------------------------------------------------------------------

if ~isempty(status)
   status = -1;
end;

return;

%===============================================================================

function status = clos1mseas (ncids,fnames);
%
% status = clos1mseas (ncids,fnames);
%
% This function closes the MSEAS PE netCDF files.
%
% ------
% Input:
% ------
%
%    ncids....Cell array of netCDF identifiers to open files.
%                NCIDS{1}  -  netCDF identifer for tracer file.
%                NCIDS{2}  -  netCDF identifer for velocity file.
%                             Can be same as tracer file.
%    fnames...Cell array of input file names.
%                FNAMES{1}  -  name of tracer file.
%                FNAMES{2}  -  name of velocity file.
%                              Can be same as tracer file.
%
% -------
% Output:
% -------
%
%    status...Exit status.  Empty if successful, -1 if not.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

status = [];
st1    = [];
st2    = [];

%-------------------------------------------------------------------------------
% Close files.
%-------------------------------------------------------------------------------

ncclose(ncids{1});
%st1 = ncclose(ncids{1});
%if ~isempty(st1)
%   disp (' ');
%   disp ('***Error:  CLOSMSEAS - unable to close netCDF file:');
%   disp (['           ',setstr(34),fnames{1},setstr(34)]);
%   disp (' ');
%end;

if ~strcmp(fnames{1},fnames{2})
ncclose(ncids{2});
%   st2 = ncclose(ncids{2});
%   if ~isempty(st2)
%      disp (' ');
%      disp ('***Error:  CLOSMSEAS - unable to close netCDF file:');
%      disp (['           ',setstr(34),fnames{2},setstr(34)]);
%      disp (' ');
%   end;
end;

%-------------------------------------------------------------------------------
% Set final exit status.
%-------------------------------------------------------------------------------

if ~isempty([st1 st2])
   status = -1;
end;

disp(' ');
disp ('CLOSMSEAS - The following NetCDF files have been closed:');
disp (['           ',char(34),fnames{1},char(34)]);
if ~strcmp(fnames{1},fnames{2})
disp (['           ',char(34),fnames{2},char(34)]);
end;
disp(' ');

return;

%===============================================================================

function [domt,xt,it,yt,jt,zt,tt,domv,xv,iv,yv,jv,zv,tv] =  ...
                                   getpepos (lon,lat,depth,time,domdat,timdat);
%
% [domt,xt,it,yt,jt,zt,tt,domv,xv,iv,yv,jv,zv,tv] =  ...
%                                  getpepos (lon,lat,depth,time,domdat,timdat);
%
% This function converts lon/lat/depth/time data into PE positions.

%-------------------------------------------------------------------------------
% Fix longitude branch.
%-------------------------------------------------------------------------------

deg2rad = pi/180;

lon = atan2(sin(lon.*deg2rad),cos(lon.*deg2rad))./deg2rad;

%clear deg2rad;

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

domt = NaN.*lon;
if (diff(size(domt))<0)
   domt = domt';
end;
xt   = domt;
it   = domt;
yt   = domt;
jt   = domt;
zt   = domt;
tt   = domt;
domv = domt;
xv   = domt;
iv   = domt;
yv   = domt;
jv   = domt;
zv   = domt;
tv   = domt;

%-------------------------------------------------------------------------------
% Loop over all domains, or until all points converted.
%-------------------------------------------------------------------------------

[ndom,ncomp] = size(domdat);

%clear ncomp;

%--------------------
%--- Tracer grid. ---
%--------------------

n = 0;

while ((n<ndom) & any(isnan(domt)))
   n = n + 1;
   ind = find (isnan(domt));
   [xw,iw,yw,jw,zw,tw] = getpe1tpos (lon(ind),lat(ind),depth(ind), ...
                                     time(ind,:),squeeze(domdat(n,:)), ...
                                     squeeze(timdat(n,:)));
   dw = n.*ones(size(ind));
   badind = find (isnan(xw));
   if ~isempty(badind)
      dw(badind) = NaN;
   end;

   domt(ind) = dw;
   xt(ind)   = xw;
   it(ind)   = iw;
   yt(ind)   = yw;
   jt(ind)   = jw;
   zt(ind)   = zw;
   tt(ind)   = tw;
end;

%----------------------
%--- Velocity grid. ---
%----------------------

n = 0;

while ((n<ndom) & any(isnan(domv)))
   n = n + 1;
   ind = find (isnan(domv));
   [xw,iw,yw,jw,zw,tw] = getpe1vpos (lon(ind),lat(ind),depth(ind), ...
                                     time(ind,:),squeeze(domdat(n,:)), ...
                                     squeeze(timdat(n,:)));
   dw = n.*ones(size(ind));
   badind = find (isnan(xw));
   if ~isempty(badind)
      dw(badind) = NaN;
   end;

   domv(ind) = dw;
   xv(ind)   = xw;
   iv(ind)   = iw;
   yv(ind)   = yw;
   jv(ind)   = jw;
   zv(ind)   = zw;
   tv(ind)   = tw;
end;

%clear n ndom ind xw iw yw jw zw tw dw badind

return;

%===============================================================================

function [xt,it,yt,jt,zt,tt] = getpe1tpos (lon,lat,depth,time,domdat,timdat);
%
% [xt,it,yt,jt,zt,tt] = getpe1tpos (lon,lat,depth,time,domdat,timdat);
%
% This function converts lon/lat/depth/time data into PE T-grid positions.

%-------------------------------------------------------------------------------
% Compute Tracer grid positions.
%-------------------------------------------------------------------------------


[xt,yt] = ll2xy (lon,lat,domdat);

zt = - depth;

tt = (datenum(time)' - timdat(1)).*timdat(3);

if (diff(size(xt))<0)
   xt = xt';
end;
if (diff(size(yt))<0)
   yt = yt';
end;
if (diff(size(zt))<0)
   zt = zt';
end;

if any(size(tt)~=size(xt))
   tt = tt';
end;

%-------------------------------------------------------------------------------
% Convert 2D positions to integral & fraction parts.
%-------------------------------------------------------------------------------

it = floor(xt);
ind = find (it>(domdat(2)-1));
if ~isempty(ind)
   it(ind) = domdat(2)-1;
end;
xt = xt - it;

jt = floor(yt);
ind = find (jt>(domdat(3)-2));
if ~isempty(ind)
   jt(ind) = domdat(3)-2;
end;
yt = yt - jt;

%-------------------------------------------------------------------------------
% Flag out-of-range locations.
%-------------------------------------------------------------------------------

ind = find ( (it<1) | (xt>1) | (jt<1) | (yt>1) | (tt<timdat(4)) | (tt>timdat(2)) );
if ~isempty(ind)
   it(ind) = NaN;
   xt(ind) = NaN;
   jt(ind) = NaN;
   yt(ind) = NaN;
   zt(ind) = NaN;
   tt(ind) = NaN;
end;

return;

%===============================================================================

function [xv,iv,yv,jv,zv,tv] = getpe1vpos (lon,lat,depth,time,domdat,timdat);
%
% [xv,iv,yv,jv,zv,tv] = getpe1vpos (lon,lat,depth,time,domdat,timdat);
%
% This function converts lon/lat/depth/time data into PE  V-grid positions.

%-------------------------------------------------------------------------------
% Compute Velocity grid positions.
%-------------------------------------------------------------------------------

[xv,yv] = ll2xy (lon,lat,domdat);
xv = xv - 0.5;
yv = yv - 0.5;

zv = - depth;

tv = (datenum(time)' - timdat(1)).*timdat(3);

if (diff(size(xv))<0)
   xv = xv';
end;
if (diff(size(yv))<0)
   yv = yv';
end;
if (diff(size(zv))<0)
   zv = zv';
end;

if any(size(tv)~=size(xv))
   tv = tv';
end;

%-------------------------------------------------------------------------------
% Convert 2D positions to integral & fraction parts.
%-------------------------------------------------------------------------------

iv = floor(xv);
ind = find (iv>(domdat(2)-2));
if ~isempty(ind)
   iv(ind) = domdat(2)-2;
end;
xv = xv - iv;

jv = floor(yv);
ind = find (jv>(domdat(3)-2));
if ~isempty(ind)
   jv(ind) = domdat(3)-2;
end;
yv = yv - jv;

%-------------------------------------------------------------------------------
% Flag out-of-range locations.
%-------------------------------------------------------------------------------

ind = find ( (iv<1) | (xv>1) | (jv<1) | (yv>1) | (tv<timdat(4)) | (tv>timdat(2)) );
if ~isempty(ind)
   iv(ind) = NaN;
   xv(ind) = NaN;
   jv(ind) = NaN;
   yv(ind) = NaN;
   zv(ind) = NaN;
   tv(ind) = NaN;
end;

return;

%===============================================================================

function vals = intrppe (varnam,ncid,fnam,vcind,dom,ipt,x,jpt,y,z,t,ny, ...
                         lndind,xtrflg);
%
% vals = intrppe (varnam,ncid,fnam,vcind,dom,ipt,x,jpt,y,z,t,ny,lndind,xtrflg);
%
% This function interpolates the given field to the requested points.
%
% ------
% Input:
% ------
%
%    varnam...Name of variable as it appears in netCDF file.
%    ncid.....Cell array of identifier(s) for opened netCDF file(s).
%    fnam.....Cell array of name of opened netCDF file(s).
%    vcind....Vector index to read.  Set to empty ([]) for scalar variables.
%    dom......Domain to use for interpolating each point.
%    ipt......Integral part of X-positions at which to interpolate.
%    x........Fractional part of X-positions at which to interpolate.
%    jpt......Integral part of Y-positions at which to interpolate.
%    y........Fractional part of Y-positions at which to interpolate.
%    z........Depths at which to interpolate PE.  (m)
%    t........Times at which to interpolate PE.   (seconds since initialization)
%    ny.......Vector of number(s) of grid points in Y-direction.
%    lndind...Cell array of indices of lands points for each domain.
%    xtrflg...Flag for types of extrapolation allowed.
%                [0] no extrapolations.
%                [1] only extrapolate up (towards the surface).
%                [2] extrapolate in both directions.  [default]
%
% -------
% Output:
% -------
%
%    vals.....Interpolated values.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

vals = [];

%-------------------------------------------------------------------------------
% If no valid points, simply return NaN vector.
%-------------------------------------------------------------------------------

if ~any(~isnan(t(:)))
   vals = t;
   return;
end;

%-------------------------------------------------------------------------------
% Loop over all unique domains, interpolating the fields.
%-------------------------------------------------------------------------------

valwk = NaN.*x;

domU = unique(dom(find(~isnan(dom))));

for n = 1:length(domU);

   d    = domU(n);
   ind  = find (~isnan(dom) & (dom==d));
   vald = intrppe1 (varnam,ncid{d},fnam{d},vcind,ipt(ind),x(ind),jpt(ind), ...
                    y(ind),z(ind),t(ind),ny(d),lndind{d},xtrflg);
   if ~isempty(vald)
      valwk(ind) = vald;
   end;

   %clear vald;

end;

vals = valwk;

%clear d domU n valwk;

return;

%===============================================================================

function vals = intrppe1 (varnam,ncid,fnam,vcind,ipt,x,jpt,y,z,t,ny,lndind, ...
                          xtrflg);
%
% vals = intrppe1 (varnam,ncid,fnam,vcind,ipt,x,jpt,y,z,t,ny,lndind,xtrflg);
%
% This function interpolates the given field to the requested points.
%
% ------
% Input:
% ------
%
%    varnam...Name of variable as it appears in netCDF file.
%    ncid.....Identifier for opened netCDF file.
%    fnam.....Name of opened netCDF file.
%    vcind....Vector index to read.  Set to empty ([]) for scalar variables.
%    ipt......Integral part of X-positions at which to interpolate.
%    x........Fractional part of X-positions at which to interpolate.
%    jpt......Integral part of Y-positions at which to interpolate.
%    y........Fractional part of Y-positions at which to interpolate.
%    z........Depths at which to interpolate PE.  (m)
%    t........Times at which to interpolate PE.   (seconds since initialization)
%    ny.......Number of grid points in Y-direction.
%    lndind...Indices of lands points.
%    xtrflg...Flag for types of extrapolation allowed.
%                [0] no extrapolations.
%                [1] only extrapolate up (towards the surface).
%                [2] extrapolate in both directions.  [default]
%
% -------
% Output:
% -------
%
%    vals.....Interpolated values.

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

vals = [];

%-------------------------------------------------------------------------------
% Get secondary data.
%-------------------------------------------------------------------------------

%------------------
%--- PE depths. ---
%------------------

znam = ncid{varnam}.positions(:).';
if isempty (znam)
   disp (' ');
   disp (['***Error:  INTRPPE - unable to read ',setstr(34),varnam, ...
          ':positions',setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),fnam,setstr(34)]);
   disp (' ');
   return;
end;

%--------------------
%--- Fill values. ---
%--------------------

vfill = ncid{varnam}.FillValue_(:);
if isempty (vfill)
   disp (' ');
   disp (['***Error:  INTRPPE - unable to read ',setstr(34),varnam, ...
          ':FillValue_',setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),fnam,setstr(34)]);
   disp (' ');
   return;
end;

%------------------
%--- PE times. ---
%------------------

name = 'time';
tpe  = ncid{name}(:);
if isempty (tpe)
   disp (' ');
   disp (['***Error:  INTRPPE - unable to read ',setstr(34),name, ...
          setstr(34),' in netCDF file:']);
   disp (['           ',setstr(34),fnam,setstr(34)]);
   disp (' ');
   return;
end;

%-------------------------------------------------------------------------------
% Determine time indices.
%-------------------------------------------------------------------------------

nt    = length(tpe);
if (nt>1)
   tindr = interp1_alt (tpe,(1:nt),t,'linear',NaN);
else
   tindr = ones(size(t));
end;

wkind  = find(~isnan(tindr));
tindlo = NaN.*tindr;
twthi  = tindlo;

if (nt>1)
   if ~isempty(wkind)
      tindlo(wkind) = floor(tindr(wkind));
      ind = find(tindlo(wkind)>(nt-1));
      if ~isempty(ind)
         tindlo(wkind(ind)) = nt-1;
      end;
   end;

   tindhi = tindlo + 1;

   if ~isempty(wkind)
      if any(size(t(wkind))~=size(tpe(tindlo(wkind))))
         twk = t(wkind)';
       else
         twk = t(wkind);
      end;

      twthi(wkind) = (twk-tpe(tindlo(wkind))) ...
                    ./ (tpe(tindhi(wkind))-tpe(tindlo(wkind)));
   end;

   twtlo = 1 - twthi;

else

   tindlo(wkind) = 1;
   tindhi(wkind) = 1;
   twthi(wkind)  = 0;
   twtlo         = 1 - twthi;

end;

%clear tpe wkind;

%-------------------------------------------------------------------------------
% Loop over all unique times, interpolating the fields.
%-------------------------------------------------------------------------------

valwk = NaN.*x;

tindloU = unique(tindlo(find(~isnan(tindlo))));

for n = 1:length(tindloU);

    tcurind = find((tindlo==tindloU(n)) & ~isnan(tindlo));

    tilo    = tindlo(tcurind(1));
    tihi    = tindhi(tcurind(1));
    twlonow = twtlo(tcurind);
    twhinow = twthi(tcurind);
    xnow    = x(tcurind);
    ynow    = y(tcurind);
    znow    = z(tcurind);
    inow    = ipt(tcurind);
    jnow    = jpt(tcurind);

    hind  = jnow + (inow-1).*ny;
    hindU = unique(hind);

    valnow = NaN.*hind;

    %---------------------------------------------------------
    %--- Loop over all unique 2x2 horizontal data squares. ---
    %---------------------------------------------------------

    for m = 1:length(hindU)

        hereind = find(hind==hindU(m));

        twlo = twlonow(hereind)';
        twhi = twhinow(hereind)';
        xhn  = xnow(hereind);
        ihn  = inow(hereind(1));
        yhn  = ynow(hereind);
        jhn  = jnow(hereind(1));
        zhn  = znow(hereind);

        if any(size(twlo)~=size(xhn))
           twlo = twlo';
        end;
        if any(size(twhi)~=size(xhn))
           twhi = twhi';
        end;

        %-----------------------
        %--- Read PE depths. ---
        %-----------------------

        zpe = squeeze(ncid{znam}(jhn:(jhn+1),ihn:(ihn+1),:,3));
        if isempty(zpe)
           disp (' ');
           disp (['***Error:  INTRPPE - unable to read ',setstr(34), ...
                  znam,setstr(34),' in netCDF file:']);
           disp (['           ',setstr(34),fnam,setstr(34)]);
           disp (' ');
           return;
        end;

        %-----------------------
        %--- Read PE values. ---
        %-----------------------

        if isempty(vcind)
           vpelo = squeeze(ncid{varnam}(tilo,jhn:(jhn+1),ihn:(ihn+1),:));
        else
           vpelo = squeeze(ncid{varnam}(tilo,jhn:(jhn+1),ihn:(ihn+1),:,vcind));
        end;
        if isempty(vpelo)
           disp (' ');
           disp (['***Error:  INTRPPE - unable to read ',setstr(34), ...
                  varnam,setstr(34),' in netCDF file:']);
           disp (['           ',setstr(34),fnam,setstr(34)]);
           disp (' ');
           return;
        end;
        ind = find (vpelo==vfill);
        if ~isempty(ind)
           vpelo(ind) = NaN;
        end;

        if isempty(vcind)
           vpehi = squeeze(ncid{varnam}(tihi,jhn:(jhn+1),ihn:(ihn+1),:));
        else
           vpehi = squeeze(ncid{varnam}(tihi,jhn:(jhn+1),ihn:(ihn+1),:,vcind));
        end;
        if isempty(vpehi)
           disp (' ');
           disp (['***Error:  INTRPPE - unable to read ',setstr(34), ...
                  varnam,setstr(34),' in netCDF file:']);
           disp (['           ',setstr(34),fnam,setstr(34)]);
           disp (' ');
           return;
        end;
        ind = find (vpehi==vfill);
        if ~isempty(ind)
           vpehi(ind) = NaN;
        end;

        %-----------------------------------------
        %--- Extract free surface conversions. ---
        %-----------------------------------------

        [etalo,xfaclo] = get_fsurf (ncid,tilo,jhn,ihn,znam);
        if isempty(xfaclo)
           disp (' ');
           disp (['***Error:  INTRPPE - unable to read free surface' ...
                  ' parameters in netCDF file:']);
           disp (['           ',setstr(34),fnam,setstr(34)]);
           disp (' ');
           return;
        end;

        [etahi,xfachi] = get_fsurf (ncid,tihi,jhn,ihn,znam);
        if isempty(xfachi)
           disp (' ');
           disp (['***Error:  INTRPPE - unable to read free surface' ...
                  ' parameters in netCDF file:']);
           disp (['           ',setstr(34),fnam,setstr(34)]);
           disp (' ');
           return;
        end;

        %-----------------------------------------------
        %--- Interpolate PE columns in the vertical. ---
        %-----------------------------------------------

        vlo = zeros(4,length(zhn));
        vhi = vlo;

        for jj = 1:2
        for ii = 1:2
           ij = ii + (jj-1)*2;
           zwk = etalo(jj,ii) + xfaclo(jj,ii).*squeeze(zpe(jj,ii,:));
           vi = interp1_alt (zwk,squeeze(vpelo(jj,ii,:)),zhn,'linear',NaN);
           ind = find(isnan(vi));
           if (~isempty(ind) & (xtrflg~=0))
              if (xtrflg==1)
                 rmind = find ( abs(zhn(ind)) > max(abs(zwk(:))) );
                 if ~isempty(rmind)
                    ind(rmind) = [];
                 end;
                 %clear rmind;
              end;
              vix = interp1_alt (zwk,squeeze(vpelo(jj,ii,:)), ...
                             zhn(ind),'nearest','extrap');
              vi(ind) = vix;
           end;
           vlo(ij,:) = vi;
           zwk = etahi(jj,ii) + xfachi(jj,ii).*squeeze(zpe(jj,ii,:));
           vi = interp1_alt (zwk,squeeze(vpehi(jj,ii,:)),zhn,'linear',NaN);
           ind = find(isnan(vi));
           if (~isempty(ind) & (xtrflg~=0))
              if (xtrflg==1)
                 rmind = find ( abs(zhn(ind)) > max(abs(zwk(:))) );
                 if ~isempty(rmind)
                    ind(rmind) = [];
                 end;
                 %clear rmind;
              end;
              vix = interp1_alt (zwk,squeeze(vpehi(jj,ii,:)), ...
                             zhn(ind),'nearest','extrap');
              vi(ind) = vix;
           end;
           vhi(ij,:) = vi;
        end;
        end;

        %--------------------------------------
        %--- Protect against masked values. ---
        %--------------------------------------

        if ~isempty(lndind)
           goodpt = zeros(1,4);

           for dj = 0:1
           for di = 0:1
               indh = (jhn+dj) + (ihn+di-1).*ny;
               ind  = di + 1 + dj*2;
               goodpt(ind) = isempty(find(lndind==indh));
           end;
           end;

           gind   = find(goodpt);
           bind   = find(~goodpt);
           ngd    = length(gind);
           indsum = sum(gind);

           if ((ngd>0) & (ngd<4))
              avlo = mean(vlo(gind,:),1);
              avhi = mean(vhi(gind,:),1);
              if (ngd==3)
                 avlo = 3.*avlo - 2.*vlo(indsum-5,:);
                 avhi = 3.*avhi - 2.*vhi(indsum-5,:);
              end;
              if ((ngd~=2) | (indsum==5));
                 for k = 1:length(bind)
                     vlo(bind(k),:) = avlo;
                     vhi(bind(k),:) = avhi;
                 end;
              else
                 for k = 1:length(bind)
                     vlo(bind(k),:) = vlo(indsum+bind(k)-5,:);
                     vhi(bind(k),:) = vhi(indsum+bind(k)-5,:);
                 end;
              end;
           elseif (ngd==0)
              vlo = vlo.*NaN;
              vhi = vhi.*NaN;
           end;
        end;

        %-------------------------------------------------
        %--- Interpolate PE columns in the horizontal. ---
        %-------------------------------------------------

        vhn = ((vlo(1,:).*(1-xhn)+vlo(2,:).*xhn).*(1-yhn) + ...
               (vlo(3,:).*(1-xhn)+vlo(4,:).*xhn).*yhn).*twlo + ...
              ((vhi(1,:).*(1-xhn)+vhi(2,:).*xhn).*(1-yhn) + ...
               (vhi(3,:).*(1-xhn)+vhi(4,:).*xhn).*yhn).*twhi;

        valnow(hereind) = squeeze(vhn);

    end;

    valwk(tcurind) = valnow;

end;

%-------------------------------------------------------------------------------
% After fully successfully processing the fields, transfer the results
% to the output variable.
%-------------------------------------------------------------------------------

vals = valwk;

return;

%===============================================================================

function [eta,xfac] = get_fsurf (ncid,tind,jind,iind,znam);
%
% [eta,xfac] = get_fsurf (ncid,tind,jind,iind,znam);
%
% This function extracts the conversions factors for depth under the free
% surface.
%
% ------
% Input:
% ------
%
%    ncid   NetCDF identifier to open MSEAS PE netCDF output file.
%    tind   Time index to extract.
%    jind   Starting j index to extract.
%    iind   Starting i index to extract.
%    znam   Name of depth variable
%
% -------
% Output:
% -------
%
%    eta    Free surface elevation.  (m)
%    xfac   Depth expansion factor.

%-------------------------------------------------------------------------------
% Quick by-pass for rigid-lid output.
%-------------------------------------------------------------------------------

if isempty(ncid{'srfpress'})
   eta  = zeros (2,2);
   xfac = ones (2,2);
   return;
end;

%-------------------------------------------------------------------------------
% Initialize output.
%-------------------------------------------------------------------------------

eta  = [];
xfac = [];

%-------------------------------------------------------------------------------
% Get raw data.
%-------------------------------------------------------------------------------

if isempty(findstr('v',znam))

   %---------------
   %--- T-grid. ---
   %---------------

   varnam = 'srfpress';
   ps = squeeze(ncid{varnam}(tind,jind:(jind+1),iind:(iind+1)));
   if isempty(ps)
      disp (' ');
      disp (['***Error:  GET_FSURF - unable to read ',setstr(34), ...
             varnam,setstr(34)]);
      disp (' ');
      return;
   end;

   varnam = 'tbath';
   hbot = squeeze(ncid{varnam}(jind:(jind+1),iind:(iind+1)));
   if isempty(hbot)
      disp (' ');
      disp (['***Error:  GET_FSURF - unable to read ',setstr(34), ...
             varnam,setstr(34)]);
      disp (' ');
      return;
   end;

else

   %---------------
   %--- V-grid. ---
   %---------------

   varnam = 'srfpress';
   pst = squeeze(ncid{varnam}(tind,jind:(jind+2),iind:(iind+2)));
   if isempty(pst)
      disp (' ');
      disp (['***Error:  GET_FSURF - unable to read ',setstr(34), ...
             varnam,setstr(34)]);
      disp (' ');
      return;
   end;

   varnam = 'tbath';
   hbott = squeeze(ncid{varnam}(jind:(jind+2),iind:(iind+2)));
   if isempty(hbott)
      disp (' ');
      disp (['***Error:  GET_FSURF - unable to read ',setstr(34), ...
             varnam,setstr(34)]);
      disp (' ');
      return;
   end;

   n = 2:3;
   s = n - 1;
   e = n;
   w = s;

   ps   = 0.25.*(pst(n,e) + pst(n,w) + pst(s,e) + pst(s,w));
   hbot = 0.25.*(hbott(n,e) + hbott(n,w) + hbott(s,e) + hbott(s,w));

   %clear n s e w pst hbott;

end

%clear varnam;

%-------------------------------------------------------------------------------
% Compute free surface and expansion factor.
%-------------------------------------------------------------------------------

cm2m = 0.01;
grav = 980.6;
p2h  = cm2m./grav;

eta = ps.*p2h;

xfac = 1 + eta./abs(hbot);

%clear cm2m grav hbot p2h ps;

return;

%===============================================================================

function pe_tim0 = get_petim0 (ncid);
%
% pe_tim0 = get_petim0 (ncid);
%
% This function extracts the initial time from a MSEAS PE output netCDF file.
%
% ------
% Input:
% ------
%
%    ncid     NetCDF identifier to open MSEAS PE netCDF output file.
%
% -------
% Output:
% -------
%
%    pe_tim0  MSEAS PE starting time vector [year month day hour minute second].

%-------------------------------------------------------------------------------
% Initialize output
%-------------------------------------------------------------------------------

pe_tim0 = [];

%-------------------------------------------------------------------------------
% Extract PE time units string
%-------------------------------------------------------------------------------

tim0str = ncid{'time'}.units(:).';

if isempty(tim0str)
   disp (' ');
   disp ('***Error:  GET_PETIM0 - unable to read time units');
   disp (' ');
   return;
end;

%-------------------------------------------------------------------------------
% Remove all but initial time data
%-------------------------------------------------------------------------------

sind = findstr (upper(tim0str),'SINCE');

if isempty(sind)
   pe_tim0 = try_pejdate0 (ncid);
   return;
end;

if (length(tim0str)<=(sind+5))
   pe_tim0 = try_pejdate0 (ncid);
   return;
end;

tim0str(1:(sind+4)) = [];

%clear sind;

tim0str = fliplr(deblank(fliplr(deblank(tim0str))));

%-------------------------------------------------------------------------------
% Convert to Gregorian data & GMT time
%-------------------------------------------------------------------------------

%--------------------------
%--- Remove Separators. ---
%--------------------------

ind = findstr(tim0str,'-');
if ~isempty(ind)
   tim0str(ind(1:2)) = ' ';
end;

ind = findstr(tim0str,':');
if ~isempty(ind)
   tim0str(ind) = ' ';
end;

%clear ind;

%---------------------------
%--- Convert to Numeric. ---
%---------------------------

twk = str2num (tim0str);

%clear tim0str

%---------------------
%--- Shift to GMT. ---
%---------------------

pe_tim0 = twk(1:6);
gmtshft = twk(7:length(twk));

%clear twk;

repind = 2:length(gmtshft);
gmtshft(repind) = abs(gmtshft(repind)).*sign(gmtshft(1));

repind = 3 + (1:length(gmtshft));
pe_tim0(repind) = pe_tim0(repind) + gmtshft;

%clear gmtshft repind;

return;

%===============================================================================

function pe_tim0 = try_pejdate0 (ncid);
%
% pe_tim0 = try_pejdate0 (ncid);
%
% This function tries extracts the initial time from a MSEAS PE output netCDF
% file, assuming the time is in modified Julian days.
%
% ------
% Input:
% ------
%
%    ncid     NetCDF identifier to open MSEAS PE netCDF output file.
%
% -------
% Output:
% -------
%
%    pe_tim0  MSEAS PE starting time vector [year month day hour minute second].

%-------------------------------------------------------------------------------
% Initialize output
%-------------------------------------------------------------------------------

pe_tim0 = [];

%-------------------------------------------------------------------------------
% Look for initial time variable in PE file.
%-------------------------------------------------------------------------------

pe_jdate0  = ncid{'time0'}(:);

if isempty(pe_tim0)
   pe_jdate0  = ncid{'dstart'}(:);
end;

if isempty(pe_jdate0)
   disp (' ');
   disp ('***Error:  TRY_PEJDATE0 - unable to extract starting time.');
   disp (' ');
   return;
end;

%-------------------------------------------------------------------------------
% Convert to Gregorian calendar.
%-------------------------------------------------------------------------------

pe_tim0 = gregorian (pe_jdate0+2440000);

%clear pe_jdate0;

return;

%===============================================================================

function [x,y] = ll2xy (lon,lat,coord,nx,ny,gridx,gridy,rlngd,rlatd, ...
                        delx,dely,thetad)
%
%=======================================================================
%                                                                    ===
% function [x,y] = ll2xy (lon,lat,coord,nx,ny,gridx,gridy,rlngd, ... ===
%                                        rlatd,delx,dely,thetad);    ===
%                                                                    ===
% This routine computes the grid coordinates from the given          ===
% longitude and latitude.                                            ===
%                                                                    ===
% ------                                                             ===
% Input:                                                             ===
% ------                                                             ===
%                                                                    ===
%     LON, LAT  the longitude and latitude of the point    (deg)     ===
%     COORD     flag for coordinate type                             ===
%                  [0]  Cartesian (tangent plane) grid.              ===
%                  [1]  unrotated spherical grid.                    ===
%                  [2]  rotated spherical grid.                      ===
%     DELX,                                                          ===
%     DELY,     Offset between transform & grid centers.   (m | deg) ===
%     GRIDX     grid spacing in x-direction                (m | deg) ===
%     GRIDY     grid spacing in y-direction                (m | deg) ===
%     NX, NY    number of grid points in x & y directions            ===
%     RLNGD     longitude at center of transformation      (deg)     ===
%     RLATD     latitude at center of transformation       (deg)     ===
%     THETAD    angle grid is rotates with respect to east (deg)     ===
%                                                                    ===
% -------                                                            ===
% Output:                                                            ===
% -------                                                            ===
%                                                                    ===
%     X, Y      the corresponding grid coordinates                   ===
%                                                                    ===
%=======================================================================

%=======================================================================
%                                                                    ===
% Calls:  rotparm                                                    ===
%                                                                    ===
%=======================================================================

if (nargin==3)
   domdat = coord;
   coord  = domdat(1);
   nx     = domdat(2);
   ny     = domdat(3);
   gridx  = domdat(4);
   gridy  = domdat(5);
   rlngd  = domdat(6);
   rlatd  = domdat(7);
   delx   = domdat(8);
   dely   = domdat(9);
   thetad = domdat(10);
   %clear domdat;
end;

deg2rad = pi/180;
xc      = (nx+1)/2 - (delx/gridx);
yc      = (ny+1)/2 - (dely/gridy);

%-----------------------------------------------------------------------
%  Cartesian (tangent plane) system.
%-----------------------------------------------------------------------

if (coord==0),
   re_d2r = 6371315.0*deg2rad;
   f1x    = re_d2r/gridx;
   f1y    = re_d2r/gridy;
   f2x    = f1x*cos(rlatd*deg2rad);
   f2y    = f1y*cos(rlatd*deg2rad);
   sterm  = sin(thetad*deg2rad);
   cterm  = cos(thetad*deg2rad);
   dlat   = lat-rlatd;
   dlon   = lon-rlngd;
   x      = xc+f1x.*dlat.*sterm+f2x.*dlon.*cterm;
   y      = yc+f1y.*dlat.*cterm-f2y.*dlon.*sterm;

%-----------------------------------------------------------------------
%  Unrotated Spherical system.
%-----------------------------------------------------------------------

  elseif (coord==1),
   x = xc+lon./gridx;
   y = yc+lat./gridy;

%-----------------------------------------------------------------------
%  Rotated spherical system.
%-----------------------------------------------------------------------

  else,
   rad2deg = 180/pi;
   [plon,plat,rnlon] = rotparm (rlngd*deg2rad,rlatd*deg2rad,thetad*deg2rad);
   cosp = cos(plat);
   sinp = sin(plat);
   cosl = cos(lat.*deg2rad);
   sinl = sin(lat.*deg2rad);
   rlat = asin(sinl.*sinp+cosl.*cosp.*cos(lon.*deg2rad-plon));
   sin1 = cosl.*sin(lon.*deg2rad-plon);
   cos1 = sinl.*cosp-sinp.*cosl.*cos(lon.*deg2rad-plon);
   rlon = rnlon-atan2(sin1,cos1);
   ind  = find(abs(rlon)>pi);
   if (~isempty(ind)),
      rlon(ind) = sign(rlon(ind)).*(rem(abs(rlon(ind)),2*pi) - ...
                             2.*pi.*fix(rem(abs(rlon(ind)),2*pi)./pi));
   end;
   rlon = rlon.*rad2deg;
   rlat = rlat.*rad2deg;
   x    = xc+rlon./gridx;
   y    = yc+rlat./gridy;

end;

return;

%===============================================================================

function [plon,plat,rnlon] = rotparm (clon,clat,rotang)
%
%=======================================================================
%                                                                    ===
% function [plon,plat,rnlon] = rotparm (clon,clat,rotang);           ===
%                                                                    ===
% This routine computes the parameters for a spherical rotation      ===
% such that the new equator passes through the center point,         ===
% making the prescribed angle with respect to East.                  ===
%                                                                    ===
% On Input:                                                          ===
%                                                                    ===
%     CLON, CLAT   The longitude and latitude of point center of the ===
%                  grid.  (radians)                                  ===
%     ROTANG       The angle of rotation with respect to East.       ===
%                  (radians)                                         ===
%                                                                    ===
% On Output:                                                         ===
%                                                                    ===
%     PLON         The longitude of the rotated pole in the original ===
%                  system.  (radians)                                ===
%     PLAT         The latitude of the rotated pole in the original  ===
%                  system.  (radians)                                ===
%     RNLON        The longitude of the original pole in the rotated ===
%                  system.  (radians)                                ===
%                                                                    ===
%=======================================================================

%=======================================================================
%                                                                    ===
% Calls:  get_ewpt                                                   ===
%                                                                    ===
%=======================================================================

qpi = pi/4;

%-----------------------------------------------------------------------
% Compute the "eastern" and "western" points along the rotated equator.
%-----------------------------------------------------------------------

[elon,elat,wlon,wlat] = get_ewpt (clon,clat,rotang);

%-----------------------------------------------------------------------
% Compute the trigonometric functions of east and west latitudes and
% of the difference of their longitudes.
%-----------------------------------------------------------------------

sine = sin(elat);
sinw = sin(wlat);
cose = cos(elat);
cosw = cos(wlat);

cosemw = cos(elon-wlon);
sinemw = sin(elon-wlon);

%-----------------------------------------------------------------------
% Compute the trigonometric functions of the interior angles.
%-----------------------------------------------------------------------

sin_pen = sinw*cose - sine*cosw*cosemw;
cos_pen = cosw*sinemw;
sin_pwn = sine*cosw - sinw*cose*cosemw;
cos_pwn = cose*sinemw;

%-----------------------------------------------------------------------
% Compute latitude of rotated pole.
%-----------------------------------------------------------------------

plat = asin( cose*cosw*sinemw );

%-----------------------------------------------------------------------
% Compute longitude of rotated pole.  Keep all longitudes in the
% range (-pi, pi].
%-----------------------------------------------------------------------

numer = sin_pen;
denom = - cos_pen*sine;
lon1 = elon + atan2(numer,denom);
lon1=sign(lon1)*(rem(abs(lon1),2*pi) - 2*pi*fix(rem(abs(lon1),2*pi)/pi));

numer = sin_pwn;
denom = - cos_pwn*sinw;
lon2 = wlon - atan2(numer,denom);
lon2=sign(lon2)*(rem(abs(lon2),2*pi) - 2*pi*fix(rem(abs(lon2),2*pi)/pi));

plon = ( lon1 + lon2 )/2;
plon=sign(plon)*(rem(abs(plon),2*pi) - 2*pi*fix(rem(abs(plon),2*pi)/pi));

%-----------------------------------------------------------------------
% Compute longitude of north pole in rotated coordinates.  Keep all
% longitudes in the range (-pi, pi].
%-----------------------------------------------------------------------

numer = sin_pen*cose;
denom = sine;
lon1 = qpi - atan2(numer,denom);
lon1=sign(lon1)*(rem(abs(lon1),2*pi) - 2*pi*fix(rem(abs(lon1),2*pi)/pi));

numer = sin_pwn*cosw;
denom = sinw;
lon2 = atan2(numer,denom) - qpi;
lon2=sign(lon2)*(rem(abs(lon2),2*pi) - 2*pi*fix(rem(abs(lon2),2*pi)/pi));

rnlon = ( lon1 + lon2 )/2;
rnlon=sign(rnlon)*(rem(abs(rnlon),2*pi)-2*pi*fix(rem(abs(rnlon),2*pi)/pi));

return;

%===============================================================================

function [elon,elat,wlon,wlat] = get_ewpt (clon,clat,rotang)
%
%=======================================================================
%                                                                    ===
% function [elon,elat,wlon,wlat] = get_ewpt (clon,clat,rotang);      ===
%                                                                    ===
% This routine computes two points to determine a spherical          ===
% rotation.  In rotated system, the points would lie on the          ===
% "equator" at "longitudes" 45E and 45W.                             ===
%                                                                    ===
% On Input:                                                          ===
%                                                                    ===
%     CLAT, CLON   The center of the grid.  (radians)                ===
%     ROTANG       The angle the new "equator" make with geographic  ===
%                  east at the center of the grid.  (radians)        ===
%                                                                    ===
% On Output:                                                         ===
%                                                                    ===
%     ELAT, ELON   The geographic coordinates of the "east point".   ===
%                  New latitude=0, New longitude=45.  (radians)      ===
%     WLAT, WLON   The geographic coordinates of the "west point".   ===
%                  New latitude=0, New longitude=-45. (radians)      ===
%                                                                    ===
%=======================================================================

%=======================================================================
%                                                                    ===
% Calls:  none                                                       ===
%                                                                    ===
%=======================================================================

rrt2 = 1/sqrt(2);
hpi = pi/2;

%-----------------------------------------------------------------------
% Find "eastern" point.
%-----------------------------------------------------------------------

colat = acos( rrt2*(sin(clat) + cos(clat)*sin(rotang)) );
elat = hpi - colat;

if ( (colat~=0) & (colat~=pi) ),

   num = rrt2*(cos(clat) - sin(clat)*sin(rotang));
   if (abs(rotang)<=pi/2),
      elon = clon + acos( num/sin(colat) );
     else,
      elon = clon - acos( num/sin(colat) );
   end;

  else

   elon = clon;

end;

%-----------------------------------------------------------------------
% Find "western" point.
%-----------------------------------------------------------------------

colat = acos( rrt2*(sin(clat) - cos(clat)*sin(rotang)) );
wlat = hpi - colat;

if ( (colat~=0) & (colat~=pi) ),

   num = rrt2*(cos(clat) + sin(clat)*sin(rotang));
   if (abs(rotang)<=pi/2),
      wlon = clon - acos( num/sin(colat) );
     else,
      wlon = clon + acos( num/sin(colat) );
   end;

  else,

   wlon = clon;

end;

%-----------------------------------------------------------------------
% Make sure latitudes are in the range [-pi/2, pi/2].
%    Step 1:  ensure that latitudes are in the range [-pi, pi].
%    Step 2:  further correct to [-pi/2, pi/2] (this involves
%             the longitudes).
% Make sure longitudes are in the range [-pi, pi].
%-----------------------------------------------------------------------

elat=sign(elat)*(rem(abs(elat),2*pi) - 2*pi*fix(rem(abs(elat),2*pi)/pi));
wlat=sign(wlat)*(rem(abs(wlat),2*pi) - 2*pi*fix(rem(abs(wlat),2*pi)/pi));

if (elat>pi/2),
   elat = pi - elat;
   elon = elon + pi;
  elseif (elat<-pi/2),
   elat = mpi - elat;
   elon = elon + pi;
end;

if (wlat>pi/2),
   wlat = pi - wlat;
   wlon = wlon + pi;
  elseif (wlat<-pi/2),
   wlat = mpi - wlat;
   wlon = wlon + pi;
end;

elon=sign(elon)*(rem(abs(elon),2*pi) - 2*pi*fix(rem(abs(elon),2*pi)/pi));
wlon=sign(wlon)*(rem(abs(wlon),2*pi) - 2*pi*fix(rem(abs(wlon),2*pi)/pi));

return;

%===============================================================================

function [gtime]=gregorian(julian)
%GREGORIAN  Converts digital Julian days to Gregorian calendar dates.
%       Although formally,
%       Julian days start and end at noon, here Julian days
%       start and end at midnight for simplicity.
%
%       In this convention, Julian day 2440000 begins at
%       0000 hours, May 23, 1968.
%
%     Usage: [gtime]=gregorian(julian)
%
%        julian... input decimal Julian day number
%
%        gtime is a six component Gregorian time vector
%          i.e.   gtime=[yyyy mo da hr mi sec]
%                 gtime=[1989 12  6  7 23 23.356]

%        yr........ year (e.g., 1979)
%        mo........ month (1-12)
%        d........ corresponding Gregorian day (1-31)
%        h........ decimal hours
%
%  calls S2HMS

      julian=julian+5.e-9;    % kludge to prevent roundoff error on seconds

%      if you want Julian Days to start at noon...
%      h=rem(julian,1)*24+12;
%      i=(h >= 24);
%      julian(i)=julian(i)+1;
%      h(i)=h(i)-24;

      secs=rem(julian,1)*24*3600;

      j = floor(julian) - 1721119;
      in = 4*j -1;
      y = floor(in/146097);
      j = in - 146097*y;
      in = floor(j/4);
      in = 4*in +3;
      j = floor(in/1461);
      d = floor(((in - 1461*j) +4)/4);
      in = 5*d -3;
      m = floor(in/153);
      d = floor(((in - 153*m) +5)/5);
      y = y*100 +j;
      mo=m-9;
      yr=y+1;
      i=(m<10);
      mo(i)=m(i)+3;
      yr(i)=y(i);
      [hour,min,sec]=s2hms(secs);
      gtime=[yr(:) mo(:) d(:) hour(:) min(:) sec(:)];

return;

%===============================================================================

function [hour,min,sec]=s2hms(secs)
%S2HMS converts seconds to integer hour,minute,seconds
%
sec=round(secs);
hour=floor(sec/3600);
min=floor(rem(sec,3600)/60);
sec=round(rem(sec,60));

return
