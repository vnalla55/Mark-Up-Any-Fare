#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013

# CCFLAGS = General options that are passed to the C and C++ compilers.
# CFLAGS = General options that are passed to the C compiler (C only; not C++).
# CXXFLAGS = General options that are passed to the C++ compiler.


ENV = dict(

    V2_LIBDIR_ = '/atse_git/fbldfunc/scons/lib',

    DEFAULT_DIST_JOBS_THRESHOLD_ = 24,
    JOBS_LIMIT_ = 300,

    BUILD_DIRS_ = [
        'Adapter',
        'AddonConstruction',
        'Allocator',
        'AppConsole',
        'ATAE',
        'Billing',
        'BookingCode',
        'BrandedFares',
        'BrandingService',
        'CacheUpdate',
        'ClientSocket',
        'Common',
        'Currency',
        'DataModel',
        'DBAccess',
        'Decode',
        'Diagnostic',
        'DSS',
        'FareCalc',
        'FareDisplay',
        'Fares',
        'FileLoader',
        'FreeBagService',
        'Internal',
        'ItinAnalyzer',
        'Limitations',
        'Manager',
        'MinFares',
        'Pricing',
        'RequestResponse',
        'RexPricing',
        'Routing',
        'RTG',
        'Rules',
        'Service',
        'ServiceFees',
        'Shopping',
        'Taxes',
        'TicketingCxrDisplay',
        'TicketingCxrService',
        'TicketingFee',
        'Util',
        'Xform',
        'Xform/CustomXMLParser',
        'Xray',
        'ZThreads',
        # The server should be processed at the end: this way
        # we can use env.stash to transport e.g. object nodes
        # which are server dependencies.
        'Server'
    ],

    TOOLS_BUILD_DIRS_ = [
        'idop_libcpp',
        'idop_libxml',
        'Allocator',
        'Tools/ldc',
        'Tools/ldc/ldc-utils'
    ],

    THIRD_PARTY_LIBDIR_ = '/opt/atseintl/adm/thirdparty',

    THIRD_PARTY_TEST_LIBDIR_ = '/opt',

    BINUTILS_DIR_ = '/opt/binutils-2.23.2-gcc-4.8.1/bin',

    # Configuration for creating all necessary objects in $BUILDDIR/bin for starting server
    #   (subdirectory_where_element_belongs_to, filename_to_link)
    SERVER_START_LINKS_ = [
        ('scons/scripts', 'acms_server.sh'),
        ('scons/scripts', 'server.sh'),

        ('scons/scripts', 'faredisplay_server.sh'),
        ('scons/scripts', 'historical_server.sh'),
        ('scons/scripts', 'pricing_server.sh'),
        ('scons/scripts', 'shopping_server.sh'),
        ('scons/scripts', 'shoppingesv_server.sh'),
        ('scons/scripts', 'shoppinghist_server.sh'),
        ('scons/scripts', 'shoppingis_server.sh'),
        ('scons/scripts', 'tax_server.sh'),

        ('scons/scripts', 'set_paths.sh'),
        ('scons/scripts', 'gettseservercfg.pl'),

        ('scons/scripts', 'dbconn.ini'),
        ('scons/scripts', 'dbaccess.ini'),
        ('scons/scripts', 'dbaccess-ora.ini'),
        ('scons/scripts', 'tnsnames.ora'),

        ('Server', 'asap.ini'),
        ('Server', 'tsi.ini'),
        ('Server', 'log4cxx.xml'),
        ('Server', 'cacheNotify.xml'),
        ('Server', 'log4j.dtd'),
        ('Server', 'sqlnet.ora'),
        ('Server', 'VIS_Beta.txt'),

        ('Xform', 'xmlConfig.cfg'),
        ('Xform', 'currencyRequest.cfg'),
        ('Xform', 'detailXmlConfig.cfg'),
        ('Xform', 'fareDisplayRequest.cfg'),
        ('Xform', 'mileageRequest.cfg'),
        ('Xform', 'pfcDisplayRequest.cfg'),
        ('Xform', 'pricingDetailRequest.cfg'),
        ('Xform', 'pricingDisplayRequest.cfg'),
        ('Xform', 'pricingRequest.cfg'),
        ('Xform', 'taxDisplayRequest.cfg'),
        ('Xform', 'taxInfoRequest.cfg'),
        ('Xform', 'taxOTARequest.cfg'),
        ('Xform', 'taxRequest.cfg')
    ],

    # Configuration groups

    BUILD_TYPES_ = {
        'debug': {
            'CCFLAGS': ['-g'],
            'BUILD_DIR_': 'sc_debug'
        },
        'release': {
            'CCFLAGS': ['-O2', '-g', '-finline-functions', '-funroll-loops', '-minline-all-stringops'],
            'CPPDEFINES': ['NDEBUG'],
            'BUILD_DIR_': 'sc_release'
        }
    },

    LINKTYPES_ = {
        'static': {
            'LIB_INSTALL_DIR_': '$STATIC_LIB_INSTALL_DIR_',
            'SERVER_APP_NAME_': 'tseserver.static'
        },
        'shared': {
            'LIB_INSTALL_DIR_': '$SHARED_LIB_INSTALL_DIR_',
            'SERVER_APP_NAME_': 'tseserver'
        }
    },

    # Compiler options

    COMPILERS_ = [
        ('gcc-*',       {'CCFLAGS': ['-fno-diagnostics-show-caret', '-Wno-error=maybe-uninitialized', '-fno-tree-vrp']}),

        ('gcc-5.2.0',   {'CXX': '${THIRD_PARTY_LIBDIR_}/gcc-5.2.0/bin/g++',
                         'STD_LIBPATH_': '${THIRD_PARTY_LIBDIR_}/gcc-5.2.0/lib64/',
                         'RPATH_': ['${THIRD_PARTY_LIBDIR_}/gcc-5.2.0/lib64']}),

        ('gcc-5.3.0',   {'CXX': '${THIRD_PARTY_LIBDIR_}/gcc-5.3.0/bin/g++',
                         'STD_LIBPATH_': '${THIRD_PARTY_LIBDIR_}/gcc-5.3.0/lib64/',
                         'RPATH_': ['${THIRD_PARTY_LIBDIR_}/gcc-5.3.0/lib64']}),

        ('clang-3.9.0', {'CXX': '${THIRD_PARTY_LIBDIR_}/llvm-3.9.0/bin/clang++',
                         'STD_LIBPATH_': '${THIRD_PARTY_LIBDIR_}/gcc-5.2.0/lib64/',
                         'CCFLAGS': ['--target=x86_64-redhat-linux-gnu'],
                         'LINKFLAGS': ['--target=x86_64-redhat-linux-gnu'],
                         'RPATH_': ['${THIRD_PARTY_LIBDIR_}/gcc-5.2.0/lib64']})
    ],

    # TODO(piotr.bartosik)
    # This should probably be assigned like that: env['ARFLAGS'] = ['rcu']
    # i.e. replacing, not appending works fine for ARFLAGS
    ARFLAGS = ['cru'],

    CCFLAGS = [
        '-B${BINUTILS_DIR_}',
        '-pipe',
        '-Wall',
        '-pthread'
    ],

    CPPDEFINES = [
        '__STDC_LIMIT_MACROS',
        'Linux',
        '_REENTRANT',
        '_POSIX_PTHREAD_SEMANTICS',
        'THREAD',
        ('_GLIBCXX_USE_CXX11_ABI', '0'),
        ('BUILD_LABEL_STRING', '"\\"' + '$BUILD_LABEL' + '\\""'),
        '_USERAWPOINTERS',
        'GTEST_PREFER_TR1_TUPLE',
        # you can change the value below whenever you switch to new library
        # and want to force scons to rebuild the objects.
        ('DUMMY_VAR','1')
    ],

    INCLUDE_PATH_ = [
        '#', '$BDB_CPPPATH_'
    ],

    ISYSTEM_PATH_ =  [
        '$BOOST_CPPPATH_',
        '$LOG4CXX_CPPPATH_',
        '$SNAPPY_CPPPATH_',
        '$LZ4_CPPPATH_',
        '$MEMCACHED_CPPPATH_',
        ],

    CPPSTD_ = 'c++14',

    CXXFLAGS = ['-std=$CPPSTD_'],

    LINKFLAGS = ['-B${BINUTILS_DIR_}', '$GOLD_LINKFLAGS_'],

    # Installing
    STATIC_LIB_INSTALL_DIR_ = '#/${BUILD_DIR_}/libstatic',
    SHARED_LIB_INSTALL_DIR_ = '#/${BUILD_DIR_}/libshared',
    LEGACY_SERVER_INSTALL_DIR_ = '#/bin/debug',
    SERVER_INSTALL_DIR_ = '#/${BUILD_DIR_}/bin',
    SERVER_INSTALL_NAME_ = '${SERVER_APP_NAME_}',

    # Gold linker

    GOLD_LINKFLAGS_ = [
        '-fuse-ld=gold',
        '-Wl,--allow-shlib-undefined',
        '-Wl,--threads',
        '-Wl,--thread-count,8'
        ],

    # Misc.

    DISABLE_DEBUG_SYMBOLS_ = dict(
      CCFLAGS = ['-g0']
    ),

    ENABLE_SYMBOLS_COMPRESSION_ = dict(
      LINKFLAGS = ['-Wl,--compress-debug-sections=zlib']
    ),

    COVERAGE_FLAG_ = dict(
      CCFLAGS = ['--coverage'],
      LINKFLAGS = ['--coverage']
    ),

    ENABLE_STDCPP_STATIC_LINK_FLAG_ = dict(
      LINKFLAGS = ['-static-libstdc++']
    ),

    EXTEND_BP_FLAG_ = dict(
      CPPDEFINES = ['EXTEND_BRANCH_PREDICTION_ON']
    ),

    WERROR_ = True,
    CACHE_DIR_ = '/project/build_storage/scons/cache',
    ISYSTEMPREFIX_ = '-isystem ',
    ISYSTEMSUFFIX_ = '',
    GENERATOR_PL_RELATIVE_PATH_ = 'test/testdata/Generator.pl',
    DIRECT_OBJECT_LINK_ = False,

    IDOPCPP_CPPPATH = '#/idop_libcpp/include',
    IDOPXML_CPPPATH = '#/idop_libxml/include',

    # External libraries info

    DEBUG_POSTFIX_ = '',

    RH_VERSION_DEPENDENT_LIBS_ = {
        6: dict(
            ASAP_DIR_ = '${THIRD_PARTY_LIBDIR_}/asap.3.0',
            ASAP_LIBLIST_ = ['asapx', 'asapw', 'readline'],

            ASAPWRAPPER_DIR_ = '${THIRD_PARTY_LIBDIR_}/asapw-2016.08.00_s',

            XERCESC_DIR_ = '${THIRD_PARTY_LIBDIR_}/xercesc-2.8.0',

            XALANC_DIR_ = '${THIRD_PARTY_LIBDIR_}/xalan',
        ),
    },

    ASAP_CPPPATH_ = '${ASAP_DIR_}/include',
    ASAP_LIBPATH_ = '${ASAP_DIR_}/lib',

    ASAPWRAPPER_CPPPATH_ = '${ASAPWRAPPER_DIR_}/include',
    ASAPWRAPPER_LIBPATH_ = '${ASAPWRAPPER_DIR_}/lib',

    XERCESC_CPPPATH_ = '${XERCESC_DIR_}/include',
    XERCESC_LIBPATH_ = '${XERCESC_DIR_}/lib',

    BDB_DIR_ = '${THIRD_PARTY_LIBDIR_}/bdb_v5.0',
    BDB_CPPPATH_ = '${BDB_DIR_}/include',
    BDB_LIBPATH_ = '${BDB_DIR_}/lib',

    BOOST_DIR_ = '${THIRD_PARTY_LIBDIR_}/boost_1_60_0${DEBUG_POSTFIX_}',
    BOOST_CPPPATH_ = '${BOOST_DIR_}/include',
    BOOST_LIBPATH_ = '${BOOST_DIR_}/lib',

    CURL_CPPPATH_ = '/usr/include/curl',

    JEMALLOC_LIBPATH_ = '${THIRD_PARTY_LIBDIR_}/jemalloc-3.4.1/lib',
    
    TBB_LIBPATH_ = '${THIRD_PARTY_LIBDIR_}/tbb40_20120408oss/lib',

    LOG4CXX_DIR_ = '${THIRD_PARTY_LIBDIR_}/apache-log4cxx-0.10.0${DEBUG_POSTFIX_}',
    LOG4CXX_CPPPATH_ = '${LOG4CXX_DIR_}/include',
    LOG4CXX_LIBPATH_ = '${LOG4CXX_DIR_}/lib',

    MEMCACHED_DIR_ = '${THIRD_PARTY_LIBDIR_}/libmemcached-0.28',
    MEMCACHED_CPPPATH_ = '${MEMCACHED_DIR_}/include',
    MEMCACHED_LIBPATH_ = '${MEMCACHED_DIR_}/lib',

    ORACLE_DIR_ = '${THIRD_PARTY_LIBDIR_}/oracle_v11.2',
    ORACLE_CPPPATH_ = '${ORACLE_DIR_}/include',
    ORACLE_LIBPATH_ = '${ORACLE_DIR_}/lib',

    SNAPPY_DIR_ = '${THIRD_PARTY_LIBDIR_}/snappy-1.1.1${DEBUG_POSTFIX_}',
    SNAPPY_CPPPATH_ = '${SNAPPY_DIR_}/include',
    SNAPPY_LIBPATH_ = '${SNAPPY_DIR_}/lib',

    LZ4_DIR_ = '${THIRD_PARTY_LIBDIR_}/lz4-r124',
    LZ4_CPPPATH_ = '${LZ4_DIR_}/include',
    LZ4_LIBPATH_ = '${LZ4_DIR_}/lib',

    CPPUNIT_DIR_ = '${THIRD_PARTY_TEST_LIBDIR_}/cppunit-1.12.1${DEBUG_POSTFIX_}',
    CPPUNIT_CPPPATH_ = '${CPPUNIT_DIR_}/include',
    CPPUNIT_LIBPATH_ = '${CPPUNIT_DIR_}/lib',

    GMOCK_DIR_ = '${THIRD_PARTY_TEST_LIBDIR_}/gmock-1.7.0',
    GMOCK_CPPPATH_ = '${GMOCK_DIR_}/include',
    GMOCK_LIBPATH_ = '${GMOCK_DIR_}/lib',

    GTEST_DIR_ = '${THIRD_PARTY_TEST_LIBDIR_}/gtest-1.7.0',
    GTEST_CPPPATH_ = '${GTEST_DIR_}/include',
    GTEST_LIBPATH_ = '${GTEST_DIR_}/lib',

    XALANC_CPPPATH_ = '${XALANC_DIR_}/include',
    XALANC_LIBPATH_ = '${XALANC_DIR_}/lib',

    EXTERNAL_LIBPATHS_ = [
        '$ASAP_LIBPATH_',
        '$ASAPWRAPPER_LIBPATH_',
        '$BDB_LIBPATH_',
        '$BOOST_LIBPATH_',
        '$JEMALLOC_LIBPATH_',
        '$TBB_LIBPATH_',
        '$LOG4CXX_LIBPATH_',
        '$MEMCACHED_LIBPATH_',
        '$ORACLE_LIBPATH_',
        '$SNAPPY_LIBPATH_',
        '$LZ4_LIBPATH_',
        '$XERCESC_LIBPATH_',
        '$XALANC_LIBPATH_'
    ],

    EXTERNAL_TEST_LIBPATHS_ = [
        '$CPPUNIT_LIBPATH_',
        '$GMOCK_LIBPATH_',
        '$GTEST_LIBPATH_'
    ],

    CXX_DEBUG_ = dict(
        CCFLAGS = ['-O2', '-fpermissive'],
        CPPDEFINES = ['_GLIBCXX_DEBUG'],
        DEBUG_POSTFIX_ = '_debug',
        DISABLE_ASAP = True,
    ),

    DISABLE_ASAP_ = dict(
        CPPDEFINES = ['TSE_DISABLE_ASAP'],
        ASAP_CPPPATH_ = '',
        ASAP_LIBPATH_ = '',
        ASAPWRAPPER_CPPPATH_ = '',
        ASAPWRAPPER_LIBPATH_ = '',
    ),

    NULL_ALLOCATOR_ = False,

    # Make the debug build optimized a bit.
    SAN_DEBUG_ = dict(
        CCFLAGS = ['-O1',
                    '-fno-omit-frame-pointer',
                    '-fno-optimize-sibling-calls']
    ),

    ASAN_ = dict(
        CXXFLAGS = ['-fsanitize=address'],
        LINKFLAGS = ['-fsanitize=address'],
        NULL_ALLOCATOR_ = True,
    ),

    UBSAN_ = dict(
        CXXFLAGS = ['-fsanitize=undefined'],
        LINKFLAGS = ['-fsanitize=undefined'],
    ),

    TSAN_ = dict(
        CXXFLAGS = ['-fsanitize=thread', '-fPIC'],
        LINKFLAGS = ['-fsanitize=thread', '-pie'],
        NULL_ALLOCATOR_ = True,
    ),

    TRACKING_ = dict(
        CPPDEFINES = ['TRACKING'],
    ),
)
