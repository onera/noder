from __future__ import annotations

import importlib.metadata
import importlib
import os
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
    "breathe",
    "exhale",
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

html_theme = "furo"
html_title = f"{project} {release}"
html_static_path = []

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

DOXYGEN_XML_DIR = os.path.join(DOCS_DIR, "_doxygen", "xml")
breathe_projects = {"noder": DOXYGEN_XML_DIR}
breathe_default_project = "noder"
breathe_domain_by_extension = {
    "hpp": "cpp",
    "h": "cpp",
}

exhale_args = {
    "containmentFolder": "./api/cpp/generated",
    "rootFileName": "library_root.rst",
    "rootFileTitle": "C++ API (Auto)",
    "doxygenStripFromPath": PROJECT_ROOT,
    "createTreeView": True,
    "exhaleExecutesDoxygen": False,
}
