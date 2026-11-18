from __future__ import annotations

import importlib
import sys
from pathlib import Path


DOCS_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = DOCS_DIR.parent
DIST_DEV = PROJECT_ROOT / "dist" / "dev"
SRC_PYTHON = PROJECT_ROOT / "src" / "python"

# Prefer the local development installation when it is available. CI and
# release builds install the wheel being documented, so avoid shadowing it
# with the source-only Python package in that case.
if DIST_DEV.exists():
    sys.path.insert(0, str(DIST_DEV))
    sys.path.insert(1, str(SRC_PYTHON))
sys.path.insert(0, str(DOCS_DIR))

try:
    noder = importlib.import_module("noder")
    importlib.import_module("noder.core")
except Exception as exc:  # pragma: no cover - docs build runtime guard
    raise RuntimeError(
        "Unable to import Noder from the local development installation or "
        "the active Python environment"
    ) from exc


project = "Noder"
author = "ONERA"
copyright = "2026, ONERA"
version = release = getattr(noder, "__version__", "dev")

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx_autodoc_typehints",
    "sphinx_copybutton",
    "doc_examples",
]

source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}
root_doc = "index"
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

html_title = f"{project} {release}"
html_theme = "pydata_sphinx_theme"
html_theme_options = {
    "logo": {
        "image_light": "_static/logos/noder_light.svg",
        "image_dark": "_static/logos/noder_dark.svg",
    },
    "navbar_start": ["navbar-logo"],
    "navbar_align": "content",
    "header_links_before_dropdown": 5,
    "navbar_end": ["theme-switcher", "navbar-icon-links"],
    "navbar_persistent": ["search-button"],
    "icon_links": [
        {
            "name": "MOLA GitLab",
            "url": "https://gitlab.onera.net/numerics/mola",
            "icon": "fa-brands fa-gitlab",
            "type": "fontawesome",
        },
        {
            "name": "GitHub",
            "url": "https://github.com/onera/noder",
            "icon": "fa-brands fa-github",
            "type": "fontawesome",
        },
    ],
    "pygments_light_style": "friendly",
    "pygments_dark_style": "monokai",
}

html_favicon = str(Path("_static/logos/n_light.svg"))
html_static_path = ["_static"]
html_css_files = [
    "css/mola-monokai-light.css",
    "css/mola-monokai-dark.css",
    "css/noder-docs.css",
]

autosummary_generate = True
autodoc_member_order = "bysource"
autodoc_default_options = {
    "member-order": "bysource",
    "show-inheritance": True,
    "special-members": "__init__",
}
autoclass_content = "both"
autodoc_typehints = "description"
autodoc_inherit_docstrings = True
always_document_param_types = True

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
