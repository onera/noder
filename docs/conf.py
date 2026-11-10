from __future__ import annotations

import importlib.metadata
import importlib
import os
from pathlib import Path
import sys

project = "noder"
copyright = "2026, ONERA"
author = "Luis Bernardos"

DOCS_DIR = os.path.abspath(os.path.dirname(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(DOCS_DIR, ".."))
DIST_DEV = os.path.join(PROJECT_ROOT, "dist", "dev")
SRC_PYTHON = os.path.join(PROJECT_ROOT, "src", "python")

if DIST_DEV not in sys.path:
    sys.path.insert(0, DIST_DEV)
if SRC_PYTHON not in sys.path:
    sys.path.append(SRC_PYTHON)

# Force-load runtime bindings from dist/dev so autodoc resolves pybind methods.
try:
    importlib.import_module("noder.core")
except Exception as exc:  # pragma: no cover - docs build runtime guard
    raise RuntimeError(f"Unable to import noder.core from {DIST_DEV}") from exc

try:
    version = release = importlib.metadata.version("noder")
except importlib.metadata.PackageNotFoundError:
    version = release = "dev"

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
    "sphinx.ext.mathjax",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx_autodoc_typehints",
    "sphinx_copybutton",
]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}
exclude_patterns = [
    "_build",
    "html",
    "html/**",
    "**.ipynb_checkpoints",
    "Thumbs.db",
    ".DS_Store",
    ".env",
    ".venv",
]


# -- Options for HTML output -------------------------------------------------
html_title = f"{project} {release}"

# The theme to use for HTML and HTML Help pages.
#
html_theme = 'pydata_sphinx_theme'

html_favicon = str(Path('_static/logos/n_light.svg'))

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {
    "logo": {
      "image_light": "_static/logos/noder_light.svg",
      "image_dark" : "_static/logos/noder_dark.svg",
    },

    "navbar_start": ["navbar-logo"],
    "navbar_align": "content",
    "header_links_before_dropdown": 5,
    'navbar_end': ['theme-switcher',
                   'navbar-icon-links',
               ],

    "navbar_persistent": ["search-button"],

    "icon_links": [

        {
            'name':'MOLA GitLab',
            'url':'https://gitlab.onera.net/numerics/mola',
            "icon": "fa-brands fa-gitlab",
            },
    ],

    "pygments_light_style": "friendly",
    "pygments_dark_style": "monokai",
}

# Add any paths that contain custom templates here, relative to this directory.
templates_path = ['_templates']

# Add any paths that contain custom static files ( such as style sheets ) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Additional CSS files for Monokai color palette (dark and light mode)
html_css_files = [
    'css/mola-monokai-light.css',
    'css/mola-monokai-dark.css',
]

autosummary_generate = True
autodoc_member_order = "bysource"
autoclass_content = "both"
autodoc_typehints = "description"
autodoc_inherit_docstrings = True

myst_enable_extensions = [
    "colon_fence",
    "deflist",
]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

nitpick_ignore = [
    ("py:class", "_io.StringIO"),
    ("py:class", "_io.BytesIO"),
]

always_document_param_types = True
