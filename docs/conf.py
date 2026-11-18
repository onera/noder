from __future__ import annotations

import importlib
import re
import subprocess
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


_STABLE_TAG_PATTERN = re.compile(
    r"^v(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)$"
)


def _get_documentation_release() -> str:
    """Return a clean stable or development version for the documentation."""
    try:
        tags = subprocess.run(
            ["git", "tag", "--list", "v[0-9]*"],
            cwd=PROJECT_ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.splitlines()
        tagged_head = subprocess.run(
            ["git", "tag", "--points-at", "HEAD"],
            cwd=PROJECT_ROOT,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.splitlines()
    except (OSError, subprocess.CalledProcessError):
        tags = []
        tagged_head = []

    stable_tags = []
    for tag in tags:
        match = _STABLE_TAG_PATTERN.fullmatch(tag)
        if match:
            version_tuple = tuple(
                int(match.group(part)) for part in ("major", "minor", "patch")
            )
            stable_tags.append((version_tuple, ".".join(map(str, version_tuple))))

    if stable_tags:
        exact_tags = []
        for tag in tagged_head:
            match = _STABLE_TAG_PATTERN.fullmatch(tag)
            if match:
                version_tuple = tuple(
                    int(match.group(part)) for part in ("major", "minor", "patch")
                )
                exact_tags.append((version_tuple, ".".join(map(str, version_tuple))))
        if exact_tags:
            return max(exact_tags)[1]

        latest_version = max(stable_tags)[0]
        return f"{latest_version[0]}.{latest_version[1]}.{latest_version[2] + 1}.dev"

    # Source archives may not include Git metadata. Normalize the packaged
    # version as a fallback and strip setuptools-scm's distance/hash suffix.
    package_version = str(getattr(noder, "__version__", "dev"))
    match = re.match(r"^(\d+\.\d+\.\d+)(\.dev\d+)?(?:\+.*)?$", package_version)
    if match:
        suffix = ".dev" if match.group(2) else ""
        return f"{match.group(1)}{suffix}"
    return "dev"


version = release = _get_documentation_release()


def _get_source_date() -> str:
    """Return the date of the latest source commit, if Git metadata exists."""
    try:
        result = subprocess.run(
            ["git", "log", "-1", "--format=%cs"],
            cwd=PROJECT_ROOT,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return "unknown"
    return result.stdout.strip() or "unknown"


myst_substitutions = {
    "noder_version": release,
    "noder_source_date": _get_source_date(),
}

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
    "substitution",
]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

nitpick_ignore = [
    ("py:class", "_io.StringIO"),
    ("py:class", "_io.BytesIO"),
]
